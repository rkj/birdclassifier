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
#include <sndfile.h>
#include <exception>
#include "mpglib/mpglib.h"
#include "mpglib/mpg123.h"

using namespace std;
typedef unsigned int uint;

const uint BUF_SIZE = 9216;
//const uint BUF_SIZE = 8192; TODO: Dlaczego to nie dziala dla MP3???

class CFile {
	public:
		CFile(const string& filename);
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
		string& getFilename(){
			return filename;
		}

	protected:
		string filename;
		int framesCount;
		uint sampleRate;
		uint channels;
		uint bufPos;
		double buffer[2*BUF_SIZE];

		virtual void fillBuffer() throw(exception) = 0;
		int readSamples;
};

class CMemoryFile : public CFile {
	public:
		double read();
		bool readPossible();
		CMemoryFile(double * mem, int size, int sampleRate, const string& filename);
		~CMemoryFile(){
		}
	protected:
		void fillBuffer() throw (exception) { 
		}
	private:
		double * data;
};

class CMP3File : public CFile {
	public:
		CMP3File(const string& filename);
		~CMP3File();
	protected:
		void fillBuffer() throw(exception);
	private:
		char fileBuffer[BUF_SIZE];
		bool needToReadFile;
		bool endOfFile;
		unsigned int bufFilled;
		short decodeBuffer[BUF_SIZE];
		mpstr mp3data;
		int size;
		int ret;
		int readFile();
		FILE* file;
};

class CWaveFile : public CFile {
	public:
		CWaveFile(const string& filename);
		~CWaveFile();
	protected:
		void fillBuffer() throw(exception);
	private:
		SF_INFO sf_info;
		SNDFILE* file;
};

class CFileFactory {
	public:
		static CFile* createCFile(const string& filename);
};
#endif
