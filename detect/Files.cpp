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


CMP3File::CMP3File(const string& filename) : CFile(filename){
	size = -1;
	InitMP3(&mp3data);
	file = fopen(filename.c_str(), "rb");
	if (file == NULL){
		fprintf(stderr, "Unable to open file: %s\n", filename.c_str());
		throw exception();
	}
	sampleRate = 44100;
	framesCount = 1;
	// while(1) {
	// 	int len = readFile();
	// 	if(len <= 0)
	// 		break;
	// 	while(ret == MP3_OK) {
	// 		int i;
	// 		framesCount += size/4;
	// 		ret = decodeMP3(&mp3data, NULL, 0, (char*)decodeBuffer, sizeof(short)*BUF_SIZE, &size);
	// 	}
	// }
	fclose(file);
	ExitMP3(&mp3data);

	memset(&mp3data, 0, sizeof(mp3data));
	readSamples = 0;
	InitMP3(&mp3data);
	file = fopen(filename.c_str(), "rb");
	if (readFile() <= 0){
		throw exception();
	}
	fflush(stderr);
	channels = 2;
	endOfFile = false;
	bufFilled = 0;
	bufPos = BUF_SIZE;
}

CMP3File::~CMP3File(){
	fclose(file);
}

int CMP3File::readFile(){
	int len = fread(fileBuffer, sizeof(unsigned char), BUF_SIZE, file);
	if (len <= 0){
		for (unsigned int i=0; i<BUF_SIZE; i++){
			buffer[i] = 0;
		}
		return len;
	}
	if (len < (int)BUF_SIZE){
		endOfFile = true;
	}
	needToReadFile = false;
	ret = decodeMP3(&mp3data, fileBuffer, len, (char*)decodeBuffer, sizeof(short)*BUF_SIZE, &size);
	return len;
}

void CMP3File::fillBuffer() throw (exception) {
//	printf("****FillBuffer (BufFilled: %d)\n", bufFilled);
	unsigned int bufFill = 0;
	if (bufFilled > 0){
		for (unsigned int i=BUF_SIZE; i<bufFilled; i++){
			buffer[i-BUF_SIZE] = 1.0*buffer[i]/SHRT_MAX;
			++bufFill;
		}
		bufFilled = 0;
	}
	while(true) {
		while (true) {
			if (bufFill >= BUF_SIZE){
				bufFilled = bufFill;
				framesCount = BUF_SIZE+1;
				return;
			}
			if (needToReadFile){
				int len = readFile();
				if (len <= 0){
					framesCount = bufFill;
					return;
				}
				// ret = decodeMP3(&mp3data, fileBuffer, len, (char*)decodeBuffer, sizeof(short)*BUF_SIZE, &size);
			} else {
				ret = decodeMP3(&mp3data, NULL, 0, (char*)decodeBuffer, sizeof(short)*BUF_SIZE, &size);
			}
			if (ret != MP3_OK){
				break;
			}
			//printf("Size: %d, read: %lld, bufFill: %d, bufFilled: %d\n", size, sampleNumber(), bufFill, bufFilled);
			if (channels == 2){
				for (int i=0; i<size/4; i++){
					buffer[bufFill++] = 1.0*decodeBuffer[2*i]/SHRT_MAX;
				}
			} else if (channels == 1) {
				for (int i=0; i<size/2; i++){
					buffer[bufFill++] = 1.0*decodeBuffer[i]/SHRT_MAX;
				}
			}
		}
		if (endOfFile){
			framesCount = bufFill;
		} 
		needToReadFile = true;
	}
}

CWaveFile::CWaveFile(const string& filename) : CFile(filename){
	sf_info.format = 0;
	file = sf_open(filename.c_str(), SFM_READ, &sf_info);
	framesCount = sf_info.frames;
	sampleRate = sf_info.samplerate;
	channels = sf_info.channels;
}

CWaveFile::~CWaveFile(){
	sf_close(file);
}

void CWaveFile::fillBuffer(){
	if (sampleRate == 44100 && channels == 1){
		sf_read_double(file, buffer, BUF_SIZE);
	} else if (sampleRate == 44100 && channels == 2){
		static double buf[2*BUF_SIZE];
		sf_read_double(file, buf, 2*BUF_SIZE);
		for (uint i=0; i<BUF_SIZE; i++){
			buffer[i] = buf[2*i];
		}
	} else {
		fprintf(stderr, "SampleRate: %d, channels: %d\n", sampleRate, channels);
		throw exception();
	}
}

CFile* CFileFactory::createCFile(const string& filename){
	if (filename.rfind(".mp3") != string::npos || filename.rfind(".MP3") != string::npos){
		return new CMP3File(filename);
	}
	if (filename.rfind(".wav") != string::npos || filename.rfind(".WAV") != string::npos){
		return new CWaveFile(filename);
	}
	return NULL;
}

