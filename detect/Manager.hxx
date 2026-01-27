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

#ifndef _MANAGER_HXX
#define _MANAGER_HXX
#include <list>
#include <memory>
#include <vector>
#include "detect.hxx"
#include "Files.hxx"
#include "Filter.hxx"


static const uint BUFSIZE = 1097152;

class CManager {
	public:
		explicit CManager(CFFT& fft);
		~CManager();
		CSample* getSample();
		void addFile(const std::string& filename);
		void resetQueue();
		void setSavePrefix(const std::string& prefix){
			savePrefix = prefix;
			lastId = 0;
		}
		//if filter is NULL nothing is done
		//else incoming data are filtered
		void setFilter(CFilter* _filter){
			filter = _filter;
		};
		void setPowerCutoff(double value){
			powerCutoff = value;
		}
		void setHopeTime(double value){
			hopeCount = (int)(44100.0*value);
		}
		uint getLastId(){
			return lastId;
		}
		void setFile(std::unique_ptr<CFile> file){
			currFile = std::move(file);
		}
	private:
		std::list<std::string> files;
		std::list<std::string> analyzedFiles;
		std::unique_ptr<CFile> currFile;

		std::vector<double> buffer;
		CFFT* fft;
		double powerCutoff;
		uint hopeCount;
		CFilter* filter;
		CSample* readFile();
		void tryToSaveSample(CSample* );
		uint lastId;
		std::string savePrefix;
};

#endif
