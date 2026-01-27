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

#include <cstdio>
#include <cstdarg>

inline void Dprintf(const char* fmt, ...) {
#ifdef _DEBUG
	va_list args;
	va_start(args, fmt);
	std::vfprintf(stderr, fmt, args);
	va_end(args);
#else
	(void)fmt;
#endif
}

#include "Audio.hxx"
#include <memory>
#ifdef QT_CORE_LIB
#include <QProgressBar>
#endif

class CManager;

void test(std::vector<CSample*>& samples, std::vector<CSample*>& learning);
CSample * test(CSample * tested, std::vector<CSample*>& learning, bool print = true);
std::vector<std::unique_ptr<CSample>> categorize(std::vector<CSample*>& samples, double delta);
void analyze(std::vector<CSample*>& samples, std::vector<CSample*>& learning);
#ifdef QT_CORE_LIB
std::vector<std::unique_ptr<CSample>> readLearning(const char* dirName, CManager& manager, QProgressBar* progress = NULL);
#else
std::vector<std::unique_ptr<CSample>> readLearning(const char* dirName, CManager& manager);
#endif
#endif
