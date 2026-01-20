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
#include <fftw3.h>

// Standard library includes (alphabetically ordered, no duplicates)
#include <cmath>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>

// Note: Do not use "using namespace std" in headers
// Use std:: prefix explicitly to avoid namespace pollution

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

// Configuration parameters for audio processing
// Use singleton pattern to provide global access while encapsulating state
struct AudioConfig {
private:
	AudioConfig() : snrMin(3.0), powerCutoff(1e-04), cutoffThreshold(0.255) {}

public:
	double snrMin;           // Signal-to-noise ratio minimum threshold
	double powerCutoff;      // Minimum power threshold for signal detection
	double cutoffThreshold;  // Maximum difference threshold for classification

	// Singleton access
	static AudioConfig& getInstance() {
		static AudioConfig instance;
		return instance;
	}

	// Delete copy and move constructors/assignments
	AudioConfig(const AudioConfig&) = delete;
	AudioConfig& operator=(const AudioConfig&) = delete;
	AudioConfig(AudioConfig&&) = delete;
	AudioConfig& operator=(AudioConfig&&) = delete;
};

// Deprecated: Use AudioConfig::getInstance().snrMin instead
// Kept for backward compatibility with existing code
extern double SNR_MIN;

char* birdPolishNameFromId(uint id);
char* birdLatinNameFromId(uint id);
char* birdShortNameFromId(uint id);
uint birdIdFromName(const std::string& name);

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
		std::string name;
		int framesCount;
		int sampleRate;
		double *frames;
		double computePower(int framesCount, int frameStart=0);
	public:
		void loadAudio (const std::string& filename);
		void saveAudio (const std::string& filename);
		void setName(std::string s){
			name = s;
		}
		const std::string& getName() const {
			return name;
		}
		const double* getFrames() const {
			return frames;
		}
		double* getFramesMutable() {
			return frames;
		}
		int getFramesCount() const {
			return framesCount;
		}
		~CSignal();
		explicit CSignal(CSignal&);
		explicit CSignal();
		explicit CSignal(const std::string& filename);
};

class CSample : public CSignal {
	public:
		void consume(CSample& other);
		double differ(CSample& other);
		void saveFrequencies(const std::string& filename);
		int saveFrequencies(FILE *);
		void saveFrequenciesTxt(const std::string& filename);
		CSample(CSample&);
		explicit CSample(const std::string& filename);
		explicit CSample(SFrequencies*, uint freqcount, uint birdid, uint sampleid);
		explicit CSample(double *, int n, uint sampleRate, uint id, uint start, uint end, uint bid);
		explicit CSample(std::vector<double>&, int start, int n, uint sampleRate, uint id, uint start, uint end, uint bid);
		~CSample();

		uint getStartSampleNo() const {
			return startSample;
		};
		uint getEndSampleNo() const {
			return endSample;
		};
		uint getId() const {
			return id;
		}
		bool IsNull() const {
			return isNull;
		}
		uint getBirdId() const {
			return birdId;
		}
		const OrigFrequencies * getOrigFrequencies() const {
			return origFrequencies;
		}
		const SFrequencies * getFrequencies() const {
			return frequencies;
		}
		uint getFreqCount() const {
			return freqCount;
		}
		std::string getBirdAndId() const {
			std::ostringstream oss;
			oss << birdPolishNameFromId(birdId) << " (" << birdId << "-" << id << ")";
			return oss.str();
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
		std::list<CSample*> samples;
	public:
		std::list<CSample*> getSamples(){
			return samples;
		}
		explicit CAudio(const std::string&);
		~CAudio();
};

int computeFrequencies(double * _in, SFrequencies **_out, int n);
int computeFrequencies(double * _in, SFrequencies **_out, OrigFrequencies ** _oOut, int n);
void saveSamples(std::vector<CSample*>& samples, std::string dir, bool frequencies);

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
void saveSamplesToFile(std::vector<CSample*>& leraning, const char* fileName);
std::vector<CSample*> readLearningFromFile(const char* filename);
#endif
