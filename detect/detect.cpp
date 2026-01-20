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

#include <sys/types.h>
#include <dirent.h>

#include "detect.hxx"
#include "Manager.hxx"

using namespace std;

bool verbose = false;
bool printUnknown = true;
bool crosstest = false;
bool applyFilter = true;
double DIF_CUTOFF = 0.255;
// double DIF_CUTOFF = 0.24;
double POWER_CUTOFF = 4e-07;

void crossTest(vector<CSample*> &samples, uint split = 10){
	if (verbose) {
		printf("Rozpoczynam cross-test\n");
	}
	random_shuffle(samples.begin(), samples.end());
	map<uint, map<uint, int> > mismatch;
	map<uint, int> match;
	map<uint, int> count;
	vector<CSample*> ns[split];
	map<CSample*, int> determinant;
	for (uint i=0; i<samples.size(); i++){
		ns[i%split].push_back(samples[i]);		
	}
	int dobrze = 0;
	int ile = 0;
	for (uint i=0; i<split; i++){
		for (uint j=0; j<ns[i].size(); j++){
			double diff = DOUBLE_BIG;
			double secondValue;
			CSample* won;
			CSample* second;
			ile++;
			for (uint k=0; k<split; k++){
				if (k!=i){
					for (uint l=0; l<ns[k].size(); l++){
						double tmp = ns[i][j]->differ(*ns[k][l]);
						if (verbose) {
							if ((k+l)%10 == 0){	//"progress bar"
								printf("\r%3d", (i+j+k+l)%100);
							}
						}
#ifdef _DEBUG
						fprintf(stderr, "%s-%s: %f\n", ns[i][j]->getName().c_str(), ns[k][l]->getName().c_str(), tmp);
#endif
						if (tmp < diff){
							if (ns[i][j]->getName() != ns[k][l]->getName()){
								secondValue = diff;
								second = ns[k][l];
							}
							diff = tmp;
							won = ns[k][l];
						}
					}
				}
			}
			const double CUTOFF = DIF_CUTOFF;
			if (won->getBirdId() == ns[i][j]->getBirdId() && diff < CUTOFF){
				determinant[won]++;
				dobrze++;
				match[won->getBirdId()]++;
				Dprintf3("Trafiony zatopiony: %d (roznica: %f)\n", won->getBirdId().c_str(), diff);
			} else {
				Dprintf4("Uuuupppsss... %s wziety za %s (roznica: %f)\n", ns[i][j]->getName().c_str(), won->getName().c_str(), diff);
				if (diff >= CUTOFF){
					mismatch[ns[i][j]->getBirdId()][0]++;
				} else {
					mismatch[ns[i][j]->getBirdId()][won->getBirdId()]++;
				}
				determinant[won]--;
			}
			count[ns[i][j]->getBirdId()]++;
		}
	}
	printf("\nCross-test results: %d/%d (%3.2f%%)\n", dobrze, ile, 100.0*dobrze/ile);
	for (map<uint, int>::iterator it = count.begin(); it != count.end(); it++){
		const char * name = birdShortNameFromId(it->first);
		int m = match[it->first];
		int all = it->second;
		printf("%s: %d/%d (%3.2f%%)\n", name, m, all, 100.0*m/all);
		for (map<uint, int>::iterator inIt = mismatch[it->first].begin(); inIt != mismatch[it->first].end(); inIt++){
			printf("  >%s: %d\n", birdShortNameFromId(inIt->first), inIt->second);
		}
	}
	vector<CSample*> learning;
	vector<CSample*> toTest;
	int wybrano = 0;
	for (map<CSample*, int>::iterator it = determinant.begin(); it != determinant.end(); it++){
		if (it->second >= 1){
			wybrano++;
			learning.push_back(it->first);
		} 
	}
	printf("Chosen: %d\n", wybrano);
	for (uint i=0; i<samples.size(); i++){
		bool copy = true;
		for (uint j=0; j<learning.size(); j++){
			if (samples[i] == learning[j]){
				copy = false;
				break;
			}
		}
		if (copy){
			toTest.push_back(samples[i]);
		}
	}
	test(toTest, learning);
	vector<CSample*> cats = *categorize(learning, 0.200);
	saveSamplesToFile(cats, "categories.freq");
	test(samples, cats);
	// saveSamples(cats, "categories/", true);
}

void test(vector<CSample*>& samples, vector<CSample*>& learning){
	printf("Beginning test\n");
	map<uint, map<uint, int> > mismatch;
	map<uint, int> birdGood;
	map<uint, int> birdCount;
	int good = 0;
	int count = 0;
	double minBad = 100.0;
	double maxGood = 0;
	for (uint i=0; i<samples.size(); i++){
		CSample* bestMatch = NULL;
		double bestValue = 100;
		const double CUTOFF = DIF_CUTOFF;
		for (uint j=0; j<learning.size(); j++){
			// double tmp = samples[i]->similar(*learning[j]);
			// if (bestValue < tmp){
			double tmp = samples[i]->differ(*learning[j]);
			if (tmp < bestValue){
				bestValue = tmp;
				bestMatch = learning[j];
			}
		}
		if (bestMatch->getBirdId() == samples[i]->getBirdId()){
			maxGood = max(maxGood, bestValue);
			++good;
		} else{
			minBad = min(minBad, bestValue);
			mismatch[samples[i]->getBirdId()][bestMatch->getBirdId()]++;
		}
		birdCount[samples[i]->getBirdId()]++;
		if (bestValue < CUTOFF) {
			birdGood[samples[i]->getBirdId()]++;
		} else {
			mismatch[samples[i]->getBirdId()][0]++;
		}
		++count;
	}
	printf("Test result: %d/%d (%3.2f%%)\n", good, count, 100.0*good/count);
	printf("MaxGood: %g; minBad: %g\n", maxGood, minBad);
	for (map<uint, map<uint, int> >::iterator it = mismatch.begin(); it != mismatch.end(); it++){
		int ile = 0;
		const char * name = birdShortNameFromId(it->first);
		int c = birdCount[it->first];
		printf("%s [%d/%d]:\n", name, birdGood[it->first], c);
		for (map<uint, int>::iterator inIt = mismatch[it->first].begin(); inIt != mismatch[it->first].end(); inIt++){
			ile+=inIt->second;
			printf("  >%s: %d (%2.2g%%)\n", birdShortNameFromId(inIt->first), inIt->second, 100.0*inIt->second/c);
		}
		printf("--- Good: %2.2g%%\n", 100.0-100.0*ile/c);
	}
}

//0.075
vector<CSample*>* categorize(vector<CSample*>& samples, double delta = 1.25){
	printf("Begining categorization\n");
	vector<CSample*> * pCategories = new vector<CSample*>();
	vector<CSample*>& categories = *pCategories;
	int blad = 0;
	for (uint i=0; i<samples.size(); i++){
		double bestValue = 100;
		CSample * bestCat = NULL;
		for (uint j=0; j<categories.size(); j++){
			if (categories[j]->getName()!=samples[i]->getName()){
				continue;
			}
			// double tmp = samples[i]->similar(*categories[j]);
			double tmp = samples[i]->differ(*categories[j]);
			// double tmp = samples[i]->correlation(*categories[j]);
			// if (tmp > bestValue){
			if (tmp < bestValue){
				bestValue = tmp;
				bestCat = categories[j];
			}
		}
		// if (bestCat == NULL || bestValue < delta){
		if (bestCat == NULL || bestValue > delta){
			categories.push_back(new CSample(*samples[i]));
		} else {
			bestCat->consume(*samples[i]);
			// if (bestCat->getName() != samples[i]->getName()){
			// 	// printf("Uuuupppsss... %s wziety za %s\n", samples[i]->getName().c_str(), bestCat->getName().c_str());
			// 	blad++;
			// } else {
			// 	// printf("Correlation: %f\n", samples[i]->correlation(*bestCat));
			// }
		}
	}
	printf("Made %d categories with %d errors\n", categories.size(), blad);
	// char buf[100];
	// for (uint i=0; i<categories.size(); i++){
	// 	sprintf(buf, "categories/%i.wav", i);
	// 	categories[i]->saveAudio(buf);
	// 	sprintf(buf, "categories/%i.freq", i);
	// 	categories[i]->saveFrequencies(buf);
	// }
	//test(samples, categories);
	return pCategories;
}

void categorize2(vector<CSample*> samples, double delta = 1.25){
	printf("Begining categorization\n");
	vector<CSample*> categories;
	int blad = 0;
	for (uint i=0; i<samples.size(); i++){
		double bestValue = 100;
		CSample * bestCat = NULL;
		for (uint j=0; j<categories.size(); j++){
			if (categories[j]->getName()!=samples[i]->getName()){
				continue;
			}
			// double tmp = samples[i]->similar(*categories[j]);
			double tmp = samples[i]->differ(*categories[j]);
			// double tmp = samples[i]->correlation(*categories[j]);
			// if (tmp > bestValue){
			if (tmp < bestValue){
				bestValue = tmp;
				bestCat = categories[j];
			}
		}
		// if (bestCat == NULL || bestValue < delta){
		if (bestCat == NULL || bestValue > delta){
			categories.push_back(new CSample(*samples[i]));
		} else {
			bestCat->consume(*samples[i]);
			// if (bestCat->getName() != samples[i]->getName()){
			// 	// printf("Uuuupppsss... %s wziety za %s\n", samples[i]->getName().c_str(), bestCat->getName().c_str());
			// 	blad++;
			// } else {
			// 	// printf("Correlation: %f\n", samples[i]->correlation(*bestCat));
			// }
		}
	}
	printf("Made %d categories with %d errors\n", categories.size(), blad);
	// char buf[100];
	// for (uint i=0; i<categories.size(); i++){
	// 	sprintf(buf, "categories/%i.wav", i);
	// 	categories[i]->saveAudio(buf);
	// 	sprintf(buf, "categories/%i.freq", i);
	// 	categories[i]->saveFrequencies(buf);
	// }
	test(samples, categories);
}

CSample * test(CSample * tested, vector<CSample*>& learning, bool print){
	CSample* bestMatch = NULL;
	double bestValue = DIF_CUTOFF;
	for (uint j=0; j<learning.size(); j++){
		double tmp = tested->differ(*learning[j]);
		if (tmp < bestValue){
			bestValue = tmp;
			bestMatch = learning[j];
		}
	}
	if (print) {
		if (bestMatch != NULL) {
			printf("%08u %04u %s %u %u %g\n", tested->getId(), bestMatch->getId(), birdShortNameFromId(bestMatch->getBirdId()), tested->getStartSampleNo(), tested->getEndSampleNo(), bestValue);
		} else {
			if (printUnknown) {
				printf("%08u 0000 UNKN %u %u %g\n", tested->getId(), tested->getStartSampleNo(), tested->getEndSampleNo(), bestValue);
			}
		}
	}
	return bestMatch;
}

void analyze(vector<CSample*>& samples, vector<CSample*>& learning){
	for (uint i=0; i<samples.size(); i++){
		test (samples[i], learning, true);
	}
}

CManager& cm = CManager::getInstance();

vector<CSample*> readLearning(const char* dirName
#ifdef QT_CORE_LIB
		, QProgressBar* progress
#endif
		){
	if (verbose)
		printf("Reading learning set\n");
	DIR *dir = opendir(dirName);
	struct dirent* de;
	while(dir){
		de = readdir(dir);
		if (de == NULL){
			break;
		}
		if (!strncmp(de->d_name, ".", 1) || !strcmp(de->d_name, ".."))
			continue;
		string filename = string(dirName)+"/"+de->d_name;
		cm.addFile(filename);
	}
	CSample* cs; 
#ifdef QT_CORE_LIB
	if (progress != NULL){
		progress->setMaximum(2000);
	}
#endif
	vector<CSample*> samples;
	for (;;){
		cs = cm.getSample();
		if (cs == NULL){
			break;
		}
		if (cs->IsNull()){
			delete cs;
			continue;
		}
		samples.push_back(cs);
#ifdef QT_CORE_LIB
		if (progress != NULL){
			progress->setValue(samples.size());
		}
#endif
		if (verbose){
			printf("."); fflush(stdout);
		}
	}
	if (verbose){
		printf("\n%d samples in learning set\n", samples.size());
	}
	return samples;
}

void analyzeDarlowo(const char* filename, vector<CSample*>& learning){
	cm.addFile(filename);
	cm.setPowerCutoff(POWER_CUTOFF);
	if (applyFilter){
		cm.setFilter(&MP3Filter);
	}
	printf("Beginning analysis of %s.\n", filename);
	vector<CSample*> testSamples;
	CSample* cs;
	for (;;){
		cs = cm.getSample();
		if (cs == NULL){
			break;
		}
		if (cs->IsNull()){
			delete cs;
			continue;
		}
		testSamples.push_back(cs);
		analyze(testSamples, learning);
		testSamples.pop_back();
		delete cs;
	}
}

void print_help(char* name){
	printf("Too few parameters. Usage:\n");
	printf("%s [-nofilter] [-snr value] [-cutoff value] [-powerCutoff value] [-hopeTime seconds] [-verbose] [-nounknown] [-save prefix] [-saveLearning prefix] [-learning dirName] [-learnFile fileName] [-crosstest | filename1 filename2...]\n", name);
}

void print_version(){
	printf("QTDetection version 0.9?\n");
	printf("Author: Roman Kamyk.\n");
	printf("Licence: GPL. See COPYING.\n");
}
int main_detect(int argc, char *argv[]){
	if (argc == 1){
		print_help(argv[0]);
		return 5;
	}
	char * saveLearning = NULL;
	char * save = NULL;
	char * dirName = "samples/";
	char * learnFile = NULL;
	vector<char*> filenames;
	for (int i = 1; i<argc; ++i){
		if (strcmp(argv[i], "-cutoff") == 0){
			if (++i == argc){
				printf("No cutoff!\n");
				return 1;
			}
			sscanf(argv[i], "%lg", &DIF_CUTOFF);
		} else if (strcmp(argv[i], "-powerCutoff") == 0){
			if (++i == argc){
				printf("No cutoff!\n");
				return 1;
			}
			sscanf(argv[i], "%lg", &POWER_CUTOFF);
		} else if (strcmp(argv[i], "-hopeTime") == 0){
			if (++i == argc){
				printf("No value!\n");
				return 1;
			}
			double tmp;
			sscanf(argv[i], "%lg", &tmp);
			cm.setHopeTime(tmp);
		} else if (strcmp(argv[i], "-snr") == 0){
			if (++i == argc){
				printf("No value!\n");
				return 1;
			}
			// Update AudioConfig singleton instead of global variable
			sscanf(argv[i], "%lg", &AudioConfig::getInstance().snrMin);
			// Also update deprecated global for backward compatibility
			SNR_MIN = AudioConfig::getInstance().snrMin;
		} else if (strcmp(argv[i], "-saveLearning") == 0){
			if (++i == argc){
				printf("No filename!\n");
				return 1;
			}
			saveLearning = argv[i];
		} else if (strcmp(argv[i], "-learnFile") == 0){
			if (++i == argc){
				printf("No filename!\n");
				return 1;
			}
			learnFile = argv[i];
		} else if (strcmp(argv[i], "-save") == 0){
			if (++i == argc){
				printf("No filename!\n");
				return 2;
			}
			save = argv[i];
		} else if (strcmp(argv[i], "-learning") == 0){
			if (++i == argc){
				printf("No filename!\n");
				return 2;
			}
			dirName = argv[i];
		} else if (strcmp(argv[i], "-verbose") == 0){
			verbose = true;
		} else if (strcmp(argv[i], "-nofilter") == 0){
			applyFilter = false;
		} else if (strcmp(argv[i], "-nounknown") == 0){
			printUnknown = false;
		} else if (strcmp(argv[i], "-crosstest") == 0){
			crosstest = true;
		} else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0){
			print_help(argv[0]);
			return 5;
		} else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0){
			print_version();
			return 6;
		} else {
			filenames.push_back(argv[i]);
		}
	}	
	vector<CSample*> learning;
	if (learnFile){
		learning = readLearningFromFile(learnFile);
	} else {
		if (saveLearning){
			cm.setSavePrefix(saveLearning);
		}
		learning = readLearning(dirName);
		if (saveLearning){
			saveSamplesToFile(learning, "learning-all.freq");
		}
	}
	cm.setSavePrefix("");
	if (crosstest){
		// crossTest(learning);
		vector<CSample*> learn = readLearningFromFile("categories.freq");
		test(learning, learn);
	}
	if (filenames.size() > 0) {
		if (save){
			cm.setSavePrefix(save);
		}
		if (verbose) {
			printf("Got %d files to analyze\n", filenames.size());
		}
		for (vector<char*>::iterator it = filenames.begin(); it != filenames.end(); ++it){
			analyzeDarlowo(*it, learning);
		}
	}
	return 0;
}
