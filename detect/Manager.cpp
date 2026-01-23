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

#include "Manager.hxx"

using namespace std;

void CManager::addFile(const string & filename){
	files.push_back(filename);
}

CManager::CManager(){
	currFile = NULL;
	buffer = new double[BUFSIZE];
	lastId = 0;
	filter = NULL;
	hopeCount = 0;
	powerCutoff = 1e-05;
	powerCutoff = 1e-04;
}

CManager::~CManager(){
	delete [] buffer;
}

CSample* CManager::readFile(){
	const uint delta = 8;
	const uint minSampleCount = 2000;
	//const uint minSampleCount = FFT_SIZE;
	const uint frontSamples = 300;
	bool rollOver = false;
	uint idx = 0;
	bool found = false;
	do {
		for (uint j=0; j<delta; j++){
			if (!currFile->readPossible()){
				return NULL;
			}
			buffer[idx+j] = currFile->read();
		}
		if (computePower(buffer+idx, delta) > powerCutoff) {
			found = true;
		}
		idx += delta;
		if (idx >= BUFSIZE-delta){
			rollOver = true;
			idx = 0;
		}
	} while (!found);

	uint pos = 0;
	if (idx < frontSamples){
		if (rollOver){
			for (int i=idx-1; i>=0; --i){
				buffer[i+frontSamples-idx] = buffer[i];
			}
			for (uint i = 0; i<frontSamples-idx; ++i){
				buffer[i] = buffer[BUFSIZE-frontSamples+idx + i];
			}
		}
		else {
			pos = idx;
		}
	} else {
		for (pos = 0; pos<frontSamples; ++pos){
			buffer[pos] = buffer[idx-frontSamples + pos];
		}
	}

	//int count = min((int)BUFSIZE, currFile->framesLeft());
	//printf("FramesLeft: %d\n", currFile->framesLeft());
	int count = (int)BUFSIZE;
	uint toLowCount = 0;
	long long startFileSample = currFile->sampleNumber();
	for (; pos<count-delta; pos+=delta){
		if (!currFile->readPossible()){
			break;
		}
		for (uint j=0; j<delta; j++){
			buffer[pos + j] = currFile->read();
			if (filter != NULL){
				buffer[pos+j] = (*filter)(buffer[pos+j]);
			}
		}
		if (computePower(buffer+pos, delta) <= powerCutoff){
			toLowCount += delta;
			if (toLowCount > hopeCount){
				break;
			}
		} else {
			toLowCount = 0;
		}
	}

	if (pos - toLowCount < minSampleCount){
		return NULL;
	}
	long long endFileSample = currFile->sampleNumber();
	const string& fn = currFile->getFilename();
	size_t last = fn.find_last_of("/");
	if (last == string::npos){
		last = 0;
	} else {
		last++;
	}
	string name = fn.substr(last, min(fn.size()-last, (size_t)4));
	CSample* sample = new CSample(buffer, pos-toLowCount, currFile->getSampleRate(), ++lastId, startFileSample, endFileSample, birdIdFromName(name));
	sample->setName(name);
	return sample;
}

void CManager::tryToSaveSample(CSample* sample){
	if (savePrefix != ""){
		char buf[20];
		sprintf(buf, "%010d.wav", lastId);
		sample->saveAudio(savePrefix + buf);
		sprintf(buf, "%010d.freq", lastId);
		sample->saveFrequencies(savePrefix + buf);
#ifdef _DEBUG
		sprintf(buf, "%010d.txt", lastId);
		sample->saveFrequenciesTxt(savePrefix + buf);
#endif
	}
}

CSample* CManager::getSample(){
	while (true){
		if (currFile == NULL){
			if (files.size() > 0){
				currFile = CFileFactory::createCFile(files.front());
				analyzedFiles.push_back(files.front());
				files.pop_front();
				continue;
			} else {
				return NULL;
			}
		}
		CSample* cs = readFile();
		if (cs != NULL){
			if (cs->IsNull()){
				delete cs;
				cs = NULL;
			} else {
				tryToSaveSample(cs);
				return cs;
			}
		}
		if (!currFile->readPossible()){
			if (release) {
				delete currFile;
			} else {
				release = true;
			}
			currFile = NULL;
		}
	}
	return NULL;
}
