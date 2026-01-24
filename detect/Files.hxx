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

#ifndef _FILE_HXX
#define _FILE_HXX

#include <sys/types.h>
#include <string>
#include <memory>
#include <sndfile.h>
#include <exception>

// Note: Do not use "using namespace std" in headers
// Use std:: prefix explicitly to avoid namespace pollution
typedef unsigned int uint;

const uint BUF_SIZE = 9216;
//const uint BUF_SIZE = 8192; TODO: Dlaczego to nie dziala dla MP3???

class CFile {
	public:
		CFile(const std::string& filename);
		virtual ~CFile() = 0;
		virtual double read();
		virtual bool readPossible();
		int sampleNumber();
		int framesLeft(){
			return framesCount;
		}
		uint getSampleRate(){
			return sampleRate;
		}
		std::string& getFilename(){
			return filename;
		}

	protected:
		std::string filename;
		int framesCount;
		uint sampleRate;
		uint channels;
		uint bufPos;
		double buffer[2*BUF_SIZE];

		virtual void fillBuffer() = 0;
		int readSamples;
};

class CMemoryFile : public CFile {
	public:
		double read();
		bool readPossible();
		CMemoryFile(double * mem, int size, int sampleRate, const std::string& filename);
		~CMemoryFile(){
		}
	protected:
		void fillBuffer() {
		}
	private:
		double * data;
};

class CWaveFile : public CFile {
	public:
		CWaveFile(const std::string& filename);
		~CWaveFile();
	protected:
		void fillBuffer();
	private:
		SF_INFO sf_info;
		SNDFILE* file;
};

class CFileFactory {
	public:
		static std::unique_ptr<CFile> createCFile(const std::string& filename);
};
#endif
