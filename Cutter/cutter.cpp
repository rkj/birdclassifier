#include <cstdio>
#include "../detect/Files.hxx"

const int SIZEOFBUF = 256*1024;

int main(int argc, char* argv[]){
	if (argc != 5){
		printf("Usage: %s filename outputFilename startSample endSample\n", argv[0]);
		exit(1);
	}
	const char *filename = argv[1];
	const char *outfn = argv[2];
	uint startSample;
	uint endSample;
	sscanf(argv[3], "%d", &startSample);
	sscanf(argv[4], "%d", &endSample);

	if (endSample < startSample){
		fprintf(stderr, "Wrong endSample < startSample!\n");
	}
	uint count = endSample-startSample+1;
	double buffer[SIZEOFBUF];
	CFile* file = NULL;
	try{
		file = CFileFactory::createCFile(filename);
		for (uint i=0; i<startSample; i++){
			if (!file->readPossible()){
				throw 1;
			}
			file->read();
			if (i % SIZEOFBUF == 0){
				printf("\rReading: %d (%d:%02d:%02d)", i, i/44100/3600, (i/44100/60)%60, (i/44100)%60);
			}
		}
		printf("\n");
		SF_INFO sfinfo;
		sfinfo.samplerate = 44100;
		sfinfo.channels = 1;
		sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
		SNDFILE* sndfile = sf_open(outfn, SFM_WRITE, &sfinfo);
		if (sndfile == NULL){
			throw 3;
		}
		for (uint i=0; i<count; ) {
			int bufPos = 0;
			while (bufPos < SIZEOFBUF && i < count){
				if (!file->readPossible()){
					count = i;
					fprintf(stderr, "Not enough samples in file\n");
					break;
				}
				buffer[bufPos++] = file->read();
				i++;
			}
			sf_write_double(sndfile, buffer, bufPos);
			printf("\rSaving: %d (%d:%02d:%02d)", i, i/44100/3600, (i/44100/60)%60, (i/44100)%60);
		}
		delete file;

		sf_close(sndfile);
		printf("\n");
	} catch (...) {
		if (file != NULL) {
			delete file;
			fprintf(stderr, "Wrong sampleNo\n");
		} else {
			fprintf(stderr, "Unable to open file\n");
		}
	}
	return 0;
}
