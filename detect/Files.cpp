/*
	QTDetection, bird voice visualization and comaprison.
	Copyright (C) 2006 Roman Kamyk.
	 
	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "Files.hxx"
#include "mpglib/mpglib.h"
#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <climits>
#include <cctype>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <memory>

using namespace std;

CFile::CFile(const string& _filename){
	this->filename = _filename;
	framesCount = 0;
	readSamples = 0;
	bufPos = BUF_SIZE;
}

CFile::~CFile(){
}

double CFile::read(){
	if (bufPos == BUF_SIZE){
		fillBuffer();
		bufPos = 0;
	}
	--framesCount;
	++readSamples;
	return buffer[bufPos++];
}

bool CFile::readPossible(){
	return framesCount > 0;
}

int CFile::sampleNumber(){
	return readSamples;
}


CMemoryFile::CMemoryFile(double *mem, int size, int sampleRate, const string & filename) : CFile(filename){
	framesCount = size;
	data = mem;
	this->sampleRate = sampleRate;
	bufPos = 0;
	readSamples = 0;
	
}

double CMemoryFile::read(){
	return data[readSamples++];
}

bool CMemoryFile::readPossible(){
	return readSamples < framesCount;
}


namespace {
constexpr int kMp3OutBufferSize = 8192;

bool bscDebugEnabled() {
	static int enabled = -1;
	if (enabled < 0) {
		const char* env = std::getenv("BSC_DEBUG");
		enabled = (env && *env) ? 1 : 0;
	}
	return enabled == 1;
}

void bscDebugLog(const char* fmt, ...) {
	if (!bscDebugEnabled()) {
		return;
	}
	va_list args;
	va_start(args, fmt);
	std::fprintf(stderr, "BSC: ");
	std::vfprintf(stderr, fmt, args);
	std::fprintf(stderr, "\n");
	va_end(args);
}

bool hasMp3Extension(const string& filename) {
	auto pos = filename.find_last_of('.');
	if (pos == string::npos) {
		return false;
	}
	string ext = filename.substr(pos + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(),
	               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return ext == "mp3";
}

size_t skipId3v2(const vector<unsigned char>& data) {
	if (data.size() < 10) {
		return 0;
	}
	if (data[0] != 'I' || data[1] != 'D' || data[2] != '3') {
		return 0;
	}
	const size_t size = ((data[6] & 0x7f) << 21) |
	                    ((data[7] & 0x7f) << 14) |
	                    ((data[8] & 0x7f) << 7) |
	                    (data[9] & 0x7f);
	size_t offset = 10 + size;
	if (data[5] & 0x10) {
		offset += 10; // footer present
	}
	return offset <= data.size() ? offset : 0;
}

int mp3SampleRateFromIndex(int index) {
	static const int kRates[] = {44100, 48000, 32000, 22050, 24000, 16000, 11025, 12000, 8000};
	if (index < 0 || index >= static_cast<int>(sizeof(kRates) / sizeof(kRates[0]))) {
		return 0;
	}
	return kRates[index];
}

class CBufferFile final : public CFile {
	public:
		CBufferFile(vector<double> samples, int sampleRate, const string& filename)
			: CFile(filename), data(std::move(samples)) {
			framesCount = static_cast<int>(data.size());
			readSamples = 0;
			bufPos = 0;
			this->sampleRate = static_cast<uint>(sampleRate);
			channels = 1;
		}

		double read() override {
			return data[readSamples++];
		}

		bool readPossible() override {
			return readSamples < framesCount;
		}

	protected:
		void fillBuffer() override {
		}

	private:
		vector<double> data;
};

unique_ptr<CFile> decodeMp3Fallback(const string& filename) {
	ifstream file(filename, ios::binary);
	if (!file) {
		throw runtime_error("Failed to open MP3 file.");
	}
	vector<unsigned char> data((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
	if (data.empty()) {
		throw runtime_error("Empty MP3 file.");
	}
	bscDebugLog("MP3 fallback decoder: loaded %zu bytes.", data.size());

	struct Mp3State {
		mpstr mp{};
		Mp3State() {
			InitMP3(&mp);
		}
		~Mp3State() {
			ExitMP3(&mp);
		}
	};

	Mp3State state;
	vector<double> samples;
	array<char, kMp3OutBufferSize> out{};
	int sampleRate = 0;
	int channels = 0;
	bool haveInfo = false;

	size_t pos = skipId3v2(data);
	if (pos > 0) {
		bscDebugLog("MP3 fallback decoder: skipped %zu bytes of ID3v2.", pos);
	}
	const size_t total = data.size();
	while (true) {
		int done = 0;
		int ret = MP3_NEED_MORE;
		if (pos < total) {
			const size_t chunk = min(static_cast<size_t>(16384), total - pos);
			ret = decodeMP3(&state.mp, reinterpret_cast<char*>(data.data() + pos),
			                static_cast<int>(chunk), out.data(), kMp3OutBufferSize, &done);
			pos += chunk;
		} else {
			ret = decodeMP3(&state.mp, nullptr, 0, out.data(), kMp3OutBufferSize, &done);
		}

		if (ret == MP3_ERR) {
			throw runtime_error("MP3 decode failed.");
		}
		if (ret == MP3_OK && done > 0) {
			if (!haveInfo) {
				sampleRate = mp3SampleRateFromIndex(state.mp.fr.sampling_frequency);
				channels = state.mp.fr.stereo;
				if (sampleRate <= 0 || channels <= 0) {
					throw runtime_error("MP3 stream metadata unavailable.");
				}
				bscDebugLog("MP3 fallback decoder: sampleRate=%d, channels=%d.", sampleRate, channels);
				haveInfo = true;
			}

			const int totalSamples = done / static_cast<int>(sizeof(short));
			const short* pcm = reinterpret_cast<const short*>(out.data());
			if (channels == 1) {
				for (int i = 0; i < totalSamples; ++i) {
					samples.push_back(static_cast<double>(pcm[i]) / 32768.0);
				}
			} else {
				for (int i = 0; i + channels - 1 < totalSamples; i += channels) {
					samples.push_back(static_cast<double>(pcm[i]) / 32768.0);
				}
			}
		}

		if (ret == MP3_NEED_MORE && pos >= total) {
			break;
		}
	}

	if (!haveInfo || samples.empty()) {
		throw runtime_error("No MP3 samples decoded.");
	}
	bscDebugLog("MP3 fallback decoder: decoded %zu samples.", samples.size());

	return make_unique<CBufferFile>(std::move(samples), sampleRate, filename);
}
} // namespace

CWaveFile::CWaveFile(const string& filename, bool emitErrors) : CFile(filename){
	sf_info.format = 0;
	file = sf_open(filename.c_str(), SFM_READ, &sf_info);
	if (!file) {
		if (emitErrors) {
			fprintf(stderr, "Error opening file '%s': %s\n", filename.c_str(), sf_strerror(NULL));
		}
		throw runtime_error("Failed to open audio file: " + filename);
	}
	framesCount = sf_info.frames;
	sampleRate = sf_info.samplerate;
	channels = sf_info.channels;
}

CWaveFile::~CWaveFile(){
	if (file)
		sf_close(file);
}

void CWaveFile::fillBuffer(){
	if (channels == 1){
		sf_count_t read = sf_read_double(file, buffer, BUF_SIZE);
		// Zero pad if we hit EOF
		for (int i = read; i < (int)BUF_SIZE; ++i) buffer[i] = 0.0;
	} else {
		// General multi-channel handling (take first channel)
		std::vector<double> multiBuf(channels * BUF_SIZE);
		sf_count_t itemsRead = sf_read_double(file, multiBuf.data(), channels * BUF_SIZE);
		sf_count_t framesRead = itemsRead / channels;

		for (int i=0; i<framesRead; i++){
			buffer[i] = multiBuf[i*channels];
		}
		// Zero pad
		for (int i = framesRead; i < (int)BUF_SIZE; ++i) buffer[i] = 0.0;
	}
}

std::unique_ptr<CFile> CFileFactory::createCFile(const string& filename){
	if (!hasMp3Extension(filename)) {
		return std::make_unique<CWaveFile>(filename);
	}

	try {
		return std::make_unique<CWaveFile>(filename, false);
	} catch (const std::exception&) {
	}
	bscDebugLog("libsndfile could not open '%s'; trying MP3 fallback.", filename.c_str());

	try {
		return decodeMp3Fallback(filename);
	} catch (const std::exception& ex) {
		fprintf(stderr, "Error opening file '%s': %s\n", filename.c_str(), ex.what());
		throw runtime_error("Failed to open audio file: " + filename);
	}
}
