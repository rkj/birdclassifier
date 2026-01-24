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

#ifndef _FILTER_HXX
#define _FILTER_HXX

#include <vector>

class CFilter{
	public:
		CFilter(int N, const double A[], const double B[]);
		~CFilter() = default;
		double operator()(double x);
		void initFilter(double data[]);
		bool initiated(){
			return vInitiated;
		};
	private:
		bool vInitiated;
		int n;
		std::vector<double> a;
		std::vector<double> b;
		std::vector<double> x;
		std::vector<double> y;
};

extern CFilter MP3Filter;
#endif
