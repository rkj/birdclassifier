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

#include "Audio.hxx"
#include "detect.hxx"
#include <array>
#include <cassert>
#include <cstring>

using namespace std;

double SNR_MIN = 3.0;

CSignal::CSignal(const string& filename){
	loadAudio(filename);
}
double CSignal::computePower(int startFrame, int framesCount){
	double energy = 0;
	for (int i=0; i<framesCount; i++){
		energy += frames[startFrame+i]*frames[startFrame+i];
	}
	return energy/framesCount;
}

void CSignal::loadAudio(const string& filename){
	SF_INFO sf_info;
	sf_info.format = 0;
	const char * fn = filename.c_str();
	SNDFILE* file = sf_open(fn, SFM_READ, &sf_info);
	if (file == NULL){
		throw string("File not found");
	}
	sampleRate = sf_info.samplerate;
	size_t numFrames = sf_info.frames;

	if (sf_info.channels == 1){
		// Mono: read directly into frames vector
		frames.resize(numFrames);
		sf_read_double(file, frames.data(), numFrames);
	} else {
		// Stereo/Multi-channel: read into temp buffer, then extract first channel
		std::vector<double> tmp(numFrames * sf_info.channels);
		sf_read_double(file, tmp.data(), numFrames);
		frames.resize(numFrames);
		for (size_t i = 0; i < numFrames; i++){
			frames[i] = tmp[sf_info.channels * i];
		}
	}
	sf_close(file);
}

void CSignal::saveAudio(const string& filename){
	SF_INFO sf_info;
	sf_info.samplerate = sampleRate;
	sf_info.channels = 1;
	sf_info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16 ;
	SNDFILE* file = sf_open(filename.c_str(), SFM_WRITE, &sf_info);
	if (file == NULL){
		throw string("Unable to create file for writing audio");
	}
	sf_write_double(file, frames.data(), frames.size());
	sf_close(file);
}

CSample::CSample(const string& filename, CFFT* fft) : CSignal(filename) {
	birdId = birdIdFromName(getName());
	if (fft != nullptr){
		computeFrequencies(*fft, frames.data(), frequencies, origFrequencies, frames.size());
	} else {
		computeFrequencies(frames.data(), frequencies, origFrequencies, frames.size());
	}
	isNull = false;
	normalize();
}

CSample::CSample(double* _frames, int n, uint _sampleRate, uint _id, uint start, uint end, uint _birdId, CFFT* fft){
	if (n <= 0){
		fprintf(stderr, "CSample: n <= 0\n");
		throw exception();
	}
	sampleRate = _sampleRate;
	frames.assign(_frames, _frames + n);
	if (fft != nullptr){
		computeFrequencies(*fft, frames.data(), frequencies, origFrequencies, frames.size());
	} else {
		computeFrequencies(frames.data(), frequencies, origFrequencies, frames.size());
	}
	isNull = false;
	normalize();
	id = _id;
	birdId = _birdId;
	startSample = start;
	endSample = end;
}

CSample::CSample(vector<double>& samples, int startS, int n, uint _sampleRate, uint _id, uint start, uint end, uint _birdId, CFFT* fft){
	if (n <= 0){
		fprintf(stderr, "CSample: n <= 0\n");
		throw exception();
	}
	sampleRate = _sampleRate;
	frames.assign(samples.begin() + startS, samples.begin() + startS + n);
	if (fft != nullptr){
		computeFrequencies(*fft, frames.data(), frequencies, origFrequencies, frames.size());
	} else {
		computeFrequencies(frames.data(), frequencies, origFrequencies, frames.size());
	}
	isNull = false;
	normalize();
	id = _id;
	birdId = _birdId;
	startSample = start;
	endSample = end;
}

CSample::CSample(const CSample& b) : CSignal(b) {
	isNull = b.isNull;
	id = b.id;
	birdId = b.birdId;
	startSample = b.startSample;
	endSample = b.endSample;
	frequencies = b.frequencies;  // vector copy
	origFrequencies = b.origFrequencies;  // vector copy
}

CSample::CSample(SFrequencies* freqs, uint freqcount, uint birdid, uint sampleid){
	// Take ownership of the raw pointer data by copying into vector, then delete
	frequencies.assign(freqs, freqs + freqcount);
	delete [] freqs;  // Clean up the raw pointer
	// Note: origFrequencies is left empty for this constructor
	this->birdId = birdid;
	this->id = sampleid;
	sampleRate = 0;
	// frames is empty (default constructed)
	isNull = false;
	startSample = 0;
	endSample = 0;
}

void CSample::consume(CSample& other){
	size_t c = min(frequencies.size(), other.frequencies.size());
	for (size_t i=0; i<c; i++){
		frequencies[i].consume(other.frequencies[i]);
	}
}

int ilosc = 0;
double CSample::differ(CSample& other){
	if (isNull || other.isNull){
		return 1.1;
	}
	size_t myFreqCount = frequencies.size();
	size_t otherFreqCount = other.frequencies.size();
	if (2 * myFreqCount < otherFreqCount || 2 * otherFreqCount < myFreqCount){
		++ilosc;
		return 1.2;
	}
	size_t count = min(myFreqCount, otherFreqCount);
	double dif = 0.0;
	for (size_t i=0; i<count; i++){
		double tmp = frequencies[i].differ(other.frequencies[i]);
		dif += tmp;
	}
	return dif/count;
}

void CSample::normalize(){
	// return;
	double maximum = -100000;
	double avg = 0;
	double minAvg = 100000;
	double minimum = 100000;
	size_t freqCount = frequencies.size();
	for (size_t j=0; j<freqCount; j++){
		double *freq = frequencies[j].freq;
		avg = 0;
		for (uint i=0; i<COUNT_FREQ; i++){
			// freq[i] = sqrt(freq[i]);
			freq[i] = log(freq[i]);
			avg += freq[i];
			if (freq[i] > maximum){
				maximum = freq[i];
			}
			if (freq[i] < minimum){
				minimum = freq[i];
			}
		}
		if (avg < minAvg){
			minAvg = avg;
		}
	}
	// minimum = -00.0f;
	minAvg /= COUNT_FREQ;
	// minimum = minAvg;
	minimum = (minAvg+maximum)/2;
	// minimum = (minAvg*2+maximum)/3;
	// printf("Max: %f, min2: %f\n", maximum, minimum);
	// printf("Max: %f, min: %f, avg: %f\n", maximum, minimum, minAvg);
	double SNR = maximum - minimum;
	// Use AudioConfig singleton instead of global SNR_MIN
	if (SNR < AudioConfig::getInstance().snrMin){
		isNull = true;
	}
	for (size_t j=0; j<freqCount; j++){
		double *freq = frequencies[j].freq;
		for (uint i=0; i<COUNT_FREQ; i++){
			// freq[i] = (freq[i]-minimum)/(maximum-minimum);
			freq[i] = max(0.0, (freq[i] - minimum)/(maximum-minimum));
			if (freq[i] > 1.0){
				printf("BLAD\n");
			}
			// freq[i] = max(0.0, freq[i]-0.2);
		}
	}
}

void CSample::saveFrequenciesTxt(const string& filename){
	FILE* file = fopen(filename.c_str(), "wb");
	if (file == NULL){
		fprintf(stderr, "Unable to create file: %s\n", filename.c_str());
		return;
	}
	for (size_t i=0; i<frequencies.size(); ++i){
		for (uint j=0; j<COUNT_FREQ; ++j){
			fprintf(file, "%e ", frequencies[i].freq[j]);
		}
		fprintf(file, "\n");
	}
	fclose(file);
}

CAudio::CAudio(const string& filename) : CSignal(filename){
	int id = 0;
	int start = -1;
	int end = -1;
	double cutoff = 1e-05;
	double analyzeTime = 0.001;
	double backTime = 0.002;
	double frontTime = 0.002;
	double minTime = 0.050;
	double hopeTime = 0.010;
	int delta = (int)(sampleRate*analyzeTime);
	delta = 1;
	int hopeCount = (int)(hopeTime * sampleRate);
	int backSamples = (int)(backTime * sampleRate);
	int frontSamples = (int)(frontTime * sampleRate);
	int minSamples = (int)(minTime * sampleRate);
	int count = 0;
	uint birdid = birdIdFromName(getName());
	size_t numFrames = frames.size();
	const int fftSize = static_cast<int>(FFT_SIZE);
	CFFT fft;

	for (size_t i = 0; i < numFrames / delta; i++){
		double power = computePower(i*delta, delta);
		if (power > cutoff) {
			if (start == -1) {
				start = i*delta;
				end = -1;
				count = 0;
			}
		} else {
			if (start != -1){
				count++;
				if (count < hopeCount){
					continue;
				}
				end = delta + i*delta + backSamples - hopeCount;
				if (end - start > minSamples){
					start = max(0, (int)(start-frontSamples));
					if (end - start < fftSize){
						end += fftSize - end + start + 1;
					}
					end = min (end + backSamples, (int)numFrames);
					samples.push_back(std::make_unique<CSample>(frames.data() + start, end-start, sampleRate, ++id, start, end, birdid, &fft));
					Dprintf3("Wycinam: %fs-%fs\n", 1.0*start/sampleRate, 1.0*end/sampleRate);
				}
				start = -1;
				end = -1;
			}
		}
	}
	if (start != -1){
		if (static_cast<int>(numFrames) - start >= fftSize){
			samples.push_back(std::make_unique<CSample>(frames.data() + start, end-start, sampleRate, ++id, start, end, birdid, &fft));
		}
	}
	Dprintf2("Sample: %s\n", filename.c_str());
	Dprintf2("Found: %d samples\n", samples.size());
	// Clear frames vector to free memory (CSample objects have their own copies)
	frames.clear();
	frames.shrink_to_fit();
}

CFFT::CFFT(){
	in = (double*)fftw_malloc(FFT_SIZE*sizeof(double));
	out = (double*)fftw_malloc(FFT_SIZE*sizeof(double));
	if (!in || !out) {
		fprintf(stderr, "Error: Failed to allocate FFTW buffers\n");
		exit(1);
	}
	// Use FFTW_ESTIMATE instead of incorrectly passing direction as flag
	rplan = fftw_plan_r2r_1d(FFT_SIZE, in, out, FFTW_R2HC, FFTW_ESTIMATE);
	if (!rplan) {
		fprintf(stderr, "Error: Failed to create FFTW plan\n");
		exit(1);
	}
}

CFFT::~CFFT(){
	fftw_free(in);
	fftw_free(out);
}


void CFFT::compute(double * _in, SFrequencies& _out, OrigFrequencies& _oOut){
	HanningWindow(_in, in, FFT_SIZE);
	fftw_execute(rplan);
	for (uint i=1; i<FFT_SIZE; ++i){
		double re = out[i];
		double im = out[FFT_SIZE-i];
		double val = re*re + im*im;
		if (i >= FIRST_FREQ && i < COUNT_FREQ+FIRST_FREQ){
			_out.freq[i-FIRST_FREQ] = val;
		}
		_oOut.freq[i] = val;
		_oOut.maxValue = max (val, _oOut.maxValue);
		_oOut.minValue = min (val, _oOut.minValue);
	}
	_oOut.freq[0] = out[0]/FFT_SIZE;
}

void CFFT::compute(double * _in, SFrequencies& _out){
	HanningWindow(_in, in, FFT_SIZE);
	fftw_execute(rplan);
	for (uint i=0; i<COUNT_FREQ; ++i){
		double re = out[i+FIRST_FREQ];
		double im = out[FFT_SIZE-FIRST_FREQ-i];
		_out.freq[i] = re*re + im*im;
	}
}

void computeFrequencies(CFFT& fft, double * _in, std::vector<SFrequencies>& _out, std::vector<OrigFrequencies>& _oOut, int n){
	int sfCount = (n-FFT_SIZE)/SEGMENT_FRAMES + 1;
	_out.resize(sfCount);
	_oOut.resize(sfCount);
	for (int i=0; i<sfCount; ++i) {
		fft.compute(_in+i*SEGMENT_FRAMES, _out[i], _oOut[i]);
	}
}

void computeFrequencies(CFFT& fft, double * _in, std::vector<SFrequencies>& _out, int n){
	int sfCount = (n-FFT_SIZE)/SEGMENT_FRAMES + 1;
	_out.resize(sfCount);
	for (int i=0; i<sfCount; ++i) {
		fft.compute(_in+i*SEGMENT_FRAMES, _out[i]);
	}
}

void computeFrequencies(double * _in, std::vector<SFrequencies>& _out, std::vector<OrigFrequencies>& _oOut, int n){
	CFFT fft;
	computeFrequencies(fft, _in, _out, _oOut, n);
}

void computeFrequencies(double * _in, std::vector<SFrequencies>& _out, int n){
	CFFT fft;
	computeFrequencies(fft, _in, _out, n);
}

SFrequencies::SFrequencies(){
}

SFrequencies::~SFrequencies(){
}

SFrequencies::SFrequencies(const SFrequencies& b){
	memcpy(freq, b.freq, sizeof(*freq)*COUNT_FREQ);
}

void SFrequencies::consume(SFrequencies&){
	// int ofc = freq_count++;
	// for (int i=0; i<CFFT::sGetFFTsize()/2; i++){
	// 	freq[i] = (freq[i]*ofc + other.freq[i])/freq_count;
	// }
}

double SFrequencies::differ(const SFrequencies& other) const {
	double dif = 0;
	int count = 0;
	for (uint i=0; i<COUNT_FREQ; i++){
		double a = min(freq[i], other.freq[i])+1;
		double b = max(freq[i], other.freq[i])+1;
		if (b > 1.0){
			dif += b/a;
			// dif += log10(b) - log10(a) + 1;
			// printf("log(%g) - log(%g) = %g - %g\n", b, a, log10(b), log10(a));
			count++;
		}
		// printf("%f / %f = %f (%f)\n", b, a, b/a, dif);
	}
	// return dif/COUNT_FREQ-1;
	return dif/count-1;
}

OrigFrequencies::OrigFrequencies(){
}

OrigFrequencies::OrigFrequencies(const OrigFrequencies & b){
	memcpy(freq, b.freq, sizeof(*freq)*FFT_SIZE);
}

OrigFrequencies::~OrigFrequencies(){
}


void saveSamples(vector<CSample*>& samples, string dir = "pociete/", bool frequencies = false){
	try{
		char buf[10];
		for (uint i=0; i<samples.size(); i++){
			sprintf(buf, "%d", i);
			const string& name = samples[i]->getName();
			string filename = dir + name + "." + buf;
			if (frequencies){
				// samples[i]->saveFrequencies(filename + ".freq");
			}
			// samples[i]->saveAudio(filename + ".wav");
		}
	} catch (string& s){
		printf("Exception: %s\n", s.c_str());
	}
}

/*
FileDescription:
FFT_SIZE - uint32
FIRST_FREQ - uint32
LAST_FREQ - uint32
frequencies amount - uint32
*/
void CSample::saveFrequencies(const string& filename){
	FILE* file = fopen(filename.c_str(), "wb");
	if (file == NULL){
		fprintf(stderr, "Unable to create file: %s\n", filename.c_str());
		return;
	}
	uint tmp = 1;
	fwrite(&FFT_SIZE, sizeof(FFT_SIZE), 1, file);
	fwrite(&FIRST_FREQ, sizeof(FIRST_FREQ), 1, file);
	fwrite(&LAST_FREQ, sizeof(LAST_FREQ), 1, file);
	fwrite(&tmp, sizeof(int), 1, file);
	if (saveFrequencies(file) != 0){
		fclose(file);
		fprintf(stderr, "Error writting to file: %s\n", filename.c_str());
		return;
	}
	fclose(file);
}
/*
FREQ_COUNT - uint32
birdid - uint32
sampleid - uint32
frequencies
*/
int CSample::saveFrequencies(FILE * file){
	uint freqCount = static_cast<uint>(frequencies.size());
	fwrite(&freqCount, sizeof(freqCount), 1, file);
	fwrite(&birdId, sizeof(birdId), 1, file);
	fwrite(&id, sizeof(id), 1, file);
	for (size_t i=0; i<frequencies.size(); ++i){
		uint freqs[COUNT_FREQ];
		for (uint j=0; j<COUNT_FREQ; ++j){
			freqs[j] = (int)(frequencies[i].freq[j]*4294967295u);
		}
		if (COUNT_FREQ != fwrite(freqs, sizeof(uint), COUNT_FREQ, file)){
			return -1;
		}
	}
	return 0;
}

void saveSamplesToFile(vector<CSample*>& learning, const char* filename){
	FILE* file = fopen(filename, "wb");
	if (file == NULL){
		fprintf(stderr, "Unable to create file: %s\n", filename);
		return;
	}
	uint tmp = learning.size();
	fwrite(&FFT_SIZE, sizeof(FFT_SIZE), 1, file);
	fwrite(&FIRST_FREQ, sizeof(FIRST_FREQ), 1, file);
	fwrite(&LAST_FREQ, sizeof(LAST_FREQ), 1, file);
	fwrite(&tmp, sizeof(int), 1, file);
	for (uint i=0; i<learning.size(); i++){
		if (learning[i]->saveFrequencies(file) != 0){
			fclose(file);
			fprintf(stderr, "Error writting to file: %s\n", filename);
			return;
		}
	}
	fclose(file);
}

vector<std::unique_ptr<CSample>> readLearningFromFile(const char* filename){
	FILE* file = fopen(filename, "rb");
	if (file == NULL){
		fprintf(stderr, "Unable to open file: %s for reading\n", filename);
		return {};
	}
	uint fft_size, first_freq, last_freq, count;
	fread(&fft_size, sizeof(FFT_SIZE), 1, file);
	fread(&first_freq, sizeof(FIRST_FREQ), 1, file);
	fread(&last_freq, sizeof(LAST_FREQ), 1, file);
	fread(&count, sizeof(uint), 1, file);
	if (FFT_SIZE != fft_size || first_freq != FIRST_FREQ || LAST_FREQ != last_freq){
		fprintf(stderr, "Wrong data format (FFT || FIRST_FREQ || LAST_FREQ differs)\n");
		fclose(file);
		return {};
	}
	vector<std::unique_ptr<CSample>> learn;
#ifdef __DEBUG__
	printf("Reading %d samples\n", count);
#endif
	for (uint i=0; i<count; i++){
		uint freqCount, birdId, id;
		fread(&freqCount, sizeof(freqCount), 1, file);
		fread(&birdId, sizeof(birdId), 1, file);
		fread(&id, sizeof(id), 1, file);
		auto frequencies = std::make_unique<SFrequencies[]>(freqCount);
		for (uint i=0; i<freqCount; ++i){
			uint freqs[COUNT_FREQ];
			if (COUNT_FREQ != fread(freqs, sizeof(uint), COUNT_FREQ, file)){
				fprintf(stderr, "Error during reading file\n");
				fclose(file);
				return learn;
			}
			for (uint j=0; j<COUNT_FREQ; ++j){
				frequencies[i].freq[j] = 1.0*freqs[j]/4294967295u;
			}
		}
		learn.push_back(std::make_unique<CSample>(frequencies.release(), freqCount, birdId, id));
	}
#ifdef __DEBUG__
	printf("Finished reading\n");
#endif
	fclose(file);
	return learn;
}

const char* birdLatinNameFromId(uint id){
	static const char *names[] = {"Unknown", "Parus Major", "Parus caeruleus", "Regulus regulus", "Erithacus rubecula"};
	if (id > 4)
		id = 0;
	return names[id];
}

const char* birdShortNameFromId(uint id){
	static const char *names[] = {"UNKN", "BOGA", "RUDZ", "MYSI", "MODR"};
	if (id > 4)
		id = 0;
	return names[id];
}

const char* birdPolishNameFromId(uint id){
	static const char *names[] = {"Nieznany", "Bogatka", "Rudzik", "Mysikrolik", "Modraszka"};
	if (id > 4)
		id = 0;
	return names[id];
}

uint birdIdFromName(const string& name){
	if (name == "BOGA"){
		return 1;
	} else if (name == "RUDZ") {
		return 2;
	} else if (name == "MYSI") {
		return 3;
	} else if (name == "MODR") {
		return 4;
	}
	return 0;
}
