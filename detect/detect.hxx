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

#ifndef _HXX_DETECT
#define _HXX_DETECT

#ifdef _DEBUG
	#define Dprintf1(x) fprintf(stderr, x);
	#define Dprintf2(x,y) fprintf(stderr, x, y)
	#define Dprintf3(x,y, z) fprintf(stderr, x, y, z)
	#define Dprintf4(x,y, z, a) fprintf(stderr, x, y, z, a)
#else
	#define Dprintf1(x) 
	#define Dprintf2(x,y)
	#define Dprintf3(x,y, z)
	#define Dprintf4(x,y, z, a) 
#endif

#include "Audio.hxx"
#ifdef QT_CORE_LIB
#include <QProgressBar>
#endif

void test(vector<CSample*>& samples, vector<CSample*>& learning);
CSample * test(CSample * tested, vector<CSample*>& learning, bool print = true);
vector<CSample*>* categorize(vector<CSample*>& samples, double delta);
void analyze(vector<CSample*>& samples, vector<CSample*>& learning);
#ifdef QT_CORE_LIB
vector<CSample*> readLearning(const char* dirName, QProgressBar* progress = NULL);
#else 
vector<CSample*> readLearning(const char* dirName);
#endif
#endif
