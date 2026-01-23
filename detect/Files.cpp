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
#include <climits>
#include <vector>

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


CWaveFile::CWaveFile(const string& filename) : CFile(filename){
	sf_info.format = 0;
	file = sf_open(filename.c_str(), SFM_READ, &sf_info);
	if (!file) {
		fprintf(stderr, "Error opening file '%s': %s\n", filename.c_str(), sf_strerror(NULL));
		throw exception();
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
		for (int i = read; i < BUF_SIZE; ++i) buffer[i] = 0.0;
	} else {
		// General multi-channel handling (take first channel)
		std::vector<double> multiBuf(channels * BUF_SIZE);
		sf_count_t itemsRead = sf_read_double(file, multiBuf.data(), channels * BUF_SIZE);
		sf_count_t framesRead = itemsRead / channels;

		for (int i=0; i<framesRead; i++){
			buffer[i] = multiBuf[i*channels];
		}
		// Zero pad
		for (int i = framesRead; i < BUF_SIZE; ++i) buffer[i] = 0.0;
	}
}

CFile* CFileFactory::createCFile(const string& filename){
	// With libsndfile 1.2.0+, MP3 and other formats are supported transparently.
	// We use CWaveFile for everything.
	return new CWaveFile(filename);
}

