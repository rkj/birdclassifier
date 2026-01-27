/*
	QTDetection, bird voice visualization and comparison.
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
#include <array>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

// Note: Do not use "using namespace std" in headers
// Use std:: prefix explicitly to avoid namespace pollution

typedef unsigned int uint;

const double PI = 3.1415926535f;
const double DOUBLE_BIG = 1e10;
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

const char* birdPolishNameFromId(uint id);
const char* birdLatinNameFromId(uint id);
const char* birdShortNameFromId(uint id);
uint birdIdFromName(const std::string& name);

template <class T>
T minim(const T& a, const T& b);

template <class T>
void HanningWindow(T* in, T* out, int n){
	// Prevent buffer overflow - window size must not exceed maximum
	assert(n > 0 && n <= 4096 && "Window size must be between 1 and 4096");

	static std::array<T, 4096> tab;
	static bool initialized = false;
	static int cached_n = 0;

	// Re-initialize if window size changes or first time
	if (!initialized || cached_n != n) {
		for (int j = 0; j < n; j++) {
			tab[j] = static_cast<T>(0.50 - 0.50 * std::cos(2 * PI * j / n));
		}
		initialized = true;
		cached_n = n;
	}

	// Apply pre-computed window
	for (int j = 0; j < n; j++) {
		out[j] = in[j] * tab[j];
	}
}

template <class T>
void HammingWindow(T* in, T* out, int n){
	// Prevent buffer overflow - window size must not exceed maximum
	assert(n > 0 && n <= 4096 && "Window size must be between 1 and 4096");

	static std::array<T, 4096> tab;
	static bool initialized = false;
	static int cached_n = 0;

	// Re-initialize if window size changes or first time
	if (!initialized || cached_n != n) {
		for (int j = 0; j < n; j++) {
			tab[j] = static_cast<T>(0.54 - 0.46 * std::cos(2 * PI * j / n));
		}
		initialized = true;
		cached_n = n;
	}

	// Apply pre-computed window
	for (int j = 0; j < n; j++) {
		out[j] = in[j] * tab[j];
	}
}

template <class T>
void BlackmanWindow(T* in, T* out, int n){
	// Prevent buffer overflow - window size must not exceed maximum
	assert(n > 0 && n <= 4096 && "Window size must be between 1 and 4096");

	static std::array<T, 4096> tab;
	static bool initialized = false;
	static int cached_n = 0;

	// Re-initialize if window size changes or first time
	if (!initialized || cached_n != n) {
		int n_1 = n - 1;
		for (int j = 0; j < n; j++) {
			tab[j] = static_cast<T>(0.42 - 0.5 * std::cos(2 * PI * j / n_1) + 0.08 * std::cos(4 * PI * j / n_1));
		}
		initialized = true;
		cached_n = n;
	}

	// Apply pre-computed window
	for (int j = 0; j < n; j++) {
		out[j] = in[j] * tab[j];
	}
}

struct SFrequencies {
	double freq[COUNT_FREQ];
	~SFrequencies();
	void consume(SFrequencies& other);
	double differ(const SFrequencies& other) const;
	SFrequencies(const SFrequencies&);
	SFrequencies();
};

struct OrigFrequencies {
	double freq[FFT_SIZE];
	~OrigFrequencies();
	OrigFrequencies();
	OrigFrequencies(const OrigFrequencies &);
	double minValue;
	double maxValue;
};

class CFFT {
	public:
		CFFT();
		~CFFT();
		int getFFTsize() const{
			return FFT_SIZE;
		}
		void compute(double * in, SFrequencies& out);
		void compute(double * _in, SFrequencies& _out, OrigFrequencies& _oOut);

	private:
		double * in;
		double * out;
		fftw_plan rplan;
};

class CSignal {
	protected:
		std::string name;
		int sampleRate;
		std::vector<double> frames;
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
		const std::vector<double>& getFrames() const {
			return frames;
		}
		std::vector<double>& getFramesMutable() {
			return frames;
		}
		size_t getFramesCount() const {
			return frames.size();
		}
		virtual ~CSignal() = default;
		CSignal() = default;
		CSignal(const CSignal&) = default;
		CSignal& operator=(const CSignal&) = default;
		CSignal(CSignal&&) noexcept = default;
		CSignal& operator=(CSignal&&) noexcept = default;
		explicit CSignal(const std::string& filename);
};

class CSample : public CSignal {
	public:
		void consume(CSample& other);
		double differ(CSample& other);
		void saveFrequencies(const std::string& filename);
		int saveFrequencies(std::ostream& out);
		void saveFrequenciesTxt(const std::string& filename);
		CSample(const CSample&);
		explicit CSample(const std::string& filename, CFFT* fft = nullptr);
		explicit CSample(SFrequencies*, uint freqcount, uint birdid, uint sampleid);
		explicit CSample(double *, int n, uint sampleRate, uint id, uint start, uint end, uint bid, CFFT* fft = nullptr);
		explicit CSample(std::vector<double>&, int startS, int n, uint sampleRate, uint id, uint start, uint end, uint bid, CFFT* fft = nullptr);
		~CSample() = default;

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
		const std::vector<OrigFrequencies>& getOrigFrequencies() const {
			return origFrequencies;
		}
		const std::vector<SFrequencies>& getFrequencies() const {
			return frequencies;
		}
		size_t getFreqCount() const {
			return frequencies.size();
		}
		std::string getBirdAndId() const {
			std::ostringstream oss;
			oss << birdPolishNameFromId(birdId) << " (" << birdId << "-" << id << ")";
			return oss.str();
		}
	private:
		bool isNull;
		std::vector<SFrequencies> frequencies;
		std::vector<OrigFrequencies> origFrequencies;
		void normalize();
		uint startSample;
		uint endSample;
		uint id;	//counted from 1
		uint birdId;
};


class CAudio : public CSignal {
	private:
		std::list<std::unique_ptr<CSample>> samples;
	public:
		std::list<CSample*> getSamples() const{
			std::list<CSample*> raw;
			for (const auto& sample : samples){
				raw.push_back(sample.get());
			}
			return raw;
		}
		explicit CAudio(const std::string&);
		~CAudio() = default;
};

void computeFrequencies(CFFT& fft, double * _in, std::vector<SFrequencies>& _out, int n);
void computeFrequencies(CFFT& fft, double * _in, std::vector<SFrequencies>& _out, std::vector<OrigFrequencies>& _oOut, int n);
void computeFrequencies(double * _in, std::vector<SFrequencies>& _out, int n);
void computeFrequencies(double * _in, std::vector<SFrequencies>& _out, std::vector<OrigFrequencies>& _oOut, int n);
void saveSamples(std::vector<CSample*>& samples, std::string dir, bool frequencies);

inline double computePower(double frames[], int framesCount){
	double energy = 0;
	for (int i=0; i<framesCount; i++){
		energy += frames[i]*frames[i];
	}
	return energy/framesCount;
}

inline double powerTodB(double value){
	return std::max(0.0, 10*log(value/10e-12)/log(10.0));
}

int main_detect(int, char**);
void saveSamplesToFile(std::vector<CSample*>& leraning, const char* fileName);
std::vector<std::unique_ptr<CSample>> readLearningFromFile(const char* filename);
#endif
