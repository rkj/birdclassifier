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

#ifndef _HXX_AUDIO
#define _HXX_AUDIO
#include <sndfile.h>
#include <map>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <list>
#include <vector>
#include <map>
#include <string>
#include <fftw3.h>

using namespace std;
typedef unsigned int uint;

const double PI = 3.1415926535f;
const double DOUBLE_BIG = 1e10;
//
// const uint FIRST_FREQ = 24;
// const uint LAST_FREQ = 128; //without it (last freq is FREQ[127]) :(
// const uint COUNT_FREQ = LAST_FREQ-FIRST_FREQ;
// const uint FFT_SIZE = 512;
// const uint SEGMENT_FRAMES = 200;
//
const uint FIRST_FREQ = 12;
const uint LAST_FREQ = 64; //without it (last freq is FREQ[63]) :(
const uint COUNT_FREQ = LAST_FREQ-FIRST_FREQ;
const uint FFT_SIZE = 256;
const uint SEGMENT_FRAMES = 100;

extern double SNR_MIN;

char* birdPolishNameFromId(uint id);
char* birdLatinNameFromId(uint id);
char* birdShortNameFromId(uint id);
uint birdIdFromName(const string& name);

template <class T>
T minim(const T& a, const T& b);
template <class T>
void HammingWindow(T* in, T* out, int n);
template <class T>
void HammingWindow(T* in, T* out, int n);
template <class T>
void BlackmanWindow(T* in, T* out, int n);

struct SFrequencies {
	double freq[COUNT_FREQ];
	~SFrequencies();
	void consume(SFrequencies& other);
	double differ(SFrequencies& other);
	SFrequencies(SFrequencies&);
	SFrequencies();
};

struct OrigFrequencies {
	double freq[FFT_SIZE];
	~OrigFrequencies();
	OrigFrequencies();
	OrigFrequencies(OrigFrequencies &);
	double minValue;
	double maxValue;
};

class CFFT {
	private:
		CFFT();
		double * in;
		double * out;
		fftw_plan rplan;

	public:
		static CFFT& getInstance(){
			static CFFT singleton;
			return singleton;
		}
		int getFFTsize(){
			return FFT_SIZE;
		}
		static int sGetFFTsize(){
			return getInstance().getFFTsize();
		}
		~CFFT();
		static void sCompute(double * in, SFrequencies& out);
		static void sCompute(double * in, SFrequencies& out, OrigFrequencies& oOut);
		void compute(double * in, SFrequencies& out);
		void compute(double * _in, SFrequencies& _out, OrigFrequencies& _oOut);
};

class CSignal {
	protected:
		string name;
		int framesCount;
		int sampleRate;
		double *frames;
		double computePower(int framesCount, int frameStart=0);
	public:
		void loadAudio (const string& filename);
		void saveAudio (const string& filename);
		void setName(string s){
			name = s;
		}
		string& getName(){
			return name;
		}
		double* getFrames(){
			return frames;
		}
		int getFramesCount(){
			return framesCount;
		}
		~CSignal();
		explicit CSignal(CSignal&);
		explicit CSignal();
		explicit CSignal(const string& filename);
};

class CSample : public CSignal {
	public:
		void consume(CSample& other);
		double differ(CSample& other);
		void saveFrequencies(const string& filename);
		int saveFrequencies(FILE *);
		void saveFrequenciesTxt(const string& filename);
		CSample(CSample&);
		explicit CSample(const string& filename);
		explicit CSample(SFrequencies*, uint freqcount, uint birdid, uint sampleid);
		explicit CSample(double *, int n, uint sampleRate, uint id, uint start, uint end, uint bid);
		explicit CSample(vector<double>&, int start, int n, uint sampleRate, uint id, uint start, uint end, uint bid);
		~CSample();

		uint getStartSampleNo(){
			return startSample;
		};
		uint getEndSampleNo(){
			return endSample;
		};
		uint getId(){
			return id;
		}
		bool IsNull(){
			return isNull;
		}
		uint getBirdId(){
			return birdId;
		}
		OrigFrequencies * getOrigFrequencies(){
			return origFrequencies;
		}
		SFrequencies * getFrequencies(){
			return frequencies;
		}
		uint getFreqCount(){
			return freqCount;
		}
		string getBirdAndId(){
			char buf[60];
			sprintf(buf, "%s (%d-%d)", birdPolishNameFromId(birdId), birdId, id);
			return buf;
		}
	private:
		bool isNull;
		uint freqCount;
		SFrequencies *frequencies;
		OrigFrequencies *origFrequencies;
		void normalize();
		uint startSample;
		uint endSample;
		uint id;	//counted from 1
		uint birdId;
};


class CAudio : public CSignal {
	private:
		list<CSample*> samples;
	public:
		list<CSample*> getSamples(){
			return samples;
		}
		explicit CAudio(const string&);
		~CAudio();
};

int computeFrequencies(double * _in, SFrequencies **_out, int n);
int computeFrequencies(double * _in, SFrequencies **_out, OrigFrequencies ** _oOut, int n);
void saveSamples(vector<CSample*>& samples, string dir, bool frequencies);

inline double computePower(double frames[], int framesCount){
	double energy = 0;
	for (int i=0; i<framesCount; i++){
		energy += frames[i]*frames[i];
	}
	return energy/framesCount;
}

inline double powerTodB(double value){
	return max(0.0, 10*log(value/10e-12)/log(10.0));
}

int main_detect(int, char**);
void saveSamplesToFile(vector<CSample*>& leraning, const char* fileName);
vector<CSample*> readLearningFromFile(const char* filename);
#endif
