/*
 * =====================================================================================
 * 
 *        Filename:  SamplesConverter.cpp
 * 
 *     Description:  Converts binary freq file to ASCII representation
 * 
 *         Version:  1.0
 *         Created:  2006-06-25 14:42:16 CEST
 *        Revision:  none
 *        Compiler:  gcc
 * 
 *          Author:   (), 
 *         Company:  
 * 
 * =====================================================================================
 */
#include <cstdio>

typedef unsigned int uint;

int main(int argc, char* argv[]){
	if (argc < 3){
		printf("Usage %s inFile.freq outFile.txt\n", argv[0]);
		return 1;
	}
	FILE* fin = fopen(argv[1], "rb");
	FILE* fout = fopen(argv[2], "w");
	if (fin == NULL || fout == NULL){
		printf("Unable to open files\n");
		return 2;
	}
	uint d1[4];
	fread(d1, sizeof(uint), 4, fin);
	fprintf(fout, "//FFTSIZE, FIRST_FREQ, LAST_FREQ, COUNT\n");
	fprintf(fout, "%d, %d, %d, %d,\n", d1[0], d1[1], d1[2], d1[3]);
	int COUNT_FREQ = d1[2] - d1[1];
	for (uint i=0; i<d1[3]; i++){
		uint freqCount, birdId, id;
		fread(&freqCount, sizeof(freqCount), 1, fin);
		fread(&birdId, sizeof(birdId), 1, fin);
		fread(&id, sizeof(id), 1, fin);
		fprintf(fout, "//Sample %d: FREQCOUNT, BIRDID, ID\n", i, freqCount, birdId, id);
		fprintf(fout, "\t%d, %d, %d, \n", freqCount, birdId, id);
		for (uint i=0; i<freqCount; ++i){
			uint freqs[COUNT_FREQ];
			if (COUNT_FREQ != fread(freqs, sizeof(uint), COUNT_FREQ, fin)){
				fprintf(stderr, "Error during reading file\n");
				return 4;
			}
			fprintf(fout, "//Freq[%d]: \n\t\t", i);
			for (uint j=0; j<COUNT_FREQ; ++j){
				fprintf(fout, "%d, ", freqs[j]);
			}
			fprintf(fout, "\n");
		}
	}
	fclose(fout);
	fclose(fin);
	return 0;
}
