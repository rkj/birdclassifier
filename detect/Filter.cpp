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

#include <cstdio>
#include <sndfile.h>
#include "Filter.hxx"


CFilter::CFilter(int N, const double A[], const double B[]){
	n = N;
	a = new double[N];
	b = new double[N];
	x = new double[N];
	y = new double[N];
	for (int i=0; i<N; ++i){
		a[i] = A[i];
		b[i] = B[i];
		x[i] = y[i] = 0;
	}
}

CFilter::~CFilter(){
	delete [] a;
	delete [] b;
	delete [] x;
	delete [] y;
}

double CFilter::operator()(double in){
	for (int i=0; i<n-1; ++i){
		x[i] = x[i+1];
		y[i] = y[i+1];
	}
	x[n-1] = in;
	y[n-1] = 0.0;
	for (int i=0; i<n; i++){
		y[n-1] += a[i] * x[n-1-i] - b[i] * y[n-1-i];
	}
	return y[n-1];
}

void CFilter::initFilter(double data[]){
	vInitiated = true;
	for (int i=0; i<n; ++i){
		(*this)(data[i]);
	}
}

const int COEF = 9;
const double a[COEF] = {0.13458, 0.00000, -0.53830, 0.00000, 0.80746, 0.00000, -0.53830, 0.00000, 0.13458};
const double b[COEF] = {1.0, -2.472581, 2.132717, -1.283293, 1.413803, -1.021015, 0.262922, -0.057071, 0.035056};

CFilter MP3Filter = CFilter(COEF, a, b);

int filter_main(int argc, char* argv[]){
	const int BUF_SIZE = 44100;

	if (argc < 3){
		printf("Usage: %s InFile OutFile\n", argv[0]);
		return 1;
	}
	int howMany = 0;
	if (argc == 4){
		sscanf(argv[3], "%d", &howMany);
	}
	SF_INFO format;
	SNDFILE* inFile = sf_open(argv[1], SFM_READ, &format);
	if (inFile == NULL){
		printf("Unable to open input file\n");
		return 1;
	}
	SNDFILE* outFile = sf_open(argv[2], SFM_WRITE, &format);
	if (outFile == NULL){
		printf("Unable to open out file\n");
		sf_close(inFile);
		return 2;
	}
	double buffer[BUF_SIZE];
	int counter = 0;
	printf("Starting processing...\n");
	while(true){
		int count = sf_read_double(inFile, buffer, BUF_SIZE);
		counter += count;
		for (int i=0; i<count; i+=format.channels){
			buffer[i] = MP3Filter(buffer[i]);
			fprintf(stderr, "buffer[%d] = %g\n", i, buffer[i]);
		}

		sf_write_double(outFile, buffer, count);
		if (count < BUF_SIZE){
			break;
		}
		if ((howMany > 0) && (counter >= howMany)){
			break;
		}
		printf(".");
		fflush(stdout);
	}
	sf_close(inFile);
	sf_close(outFile);
	printf("\nFinished processing\n");
	return 0;
}

