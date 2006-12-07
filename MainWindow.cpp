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

#include "MainWindow.hxx"
#include "LearningDialog.hxx"

#include <QMutex>
QMutex samplesMutex;
MainWindow* myWindow;

uint play_from, play_to;

const uint MAX_FRAMES = 44100/*fps*/ * 60 /*seconds*/ * 5 /*minutes*/;

int MainWindow::playRecordCallback(char *buffer, int bufferSize, void * data){
	AudioDraw* asd = myWindow->audioSignalDraw->getAudioDraw();
	MainWindow* that = myWindow;
	if (that->playing){
		if (play_from >= play_to){
			asd->setMarker(-1);
			return 1;
		}
		double * b = (double *) buffer;
		vector<double>& d = *(vector<double>*)data;
		int count = min(bufferSize, (int)(play_to - play_from));
		for (int i=0; i<count; ++i){
			b[i] = d[play_from++];
			if (i % 100 == 0){
			}
		}
		asd->setMarker(play_from);
	} else if (that->recording){
		double * b = (double *) buffer;
		samplesMutex.lock();
		for (int i=0; i<bufferSize; ++i){
			that->samples.push_back(b[i]);
			that->fSamples.push_back(MP3Filter(b[i]));
			b[i] = 0;
		}
		samplesMutex.unlock();
		that->audioSignalDraw->setSignal(that->samples);
		that->filteredDraw->setSignal(that->fSamples);
		// that->energyDraw->getEnergy()->updateSignal(that->fSamples);
		if (that->samples.size() > 44100 * 60){
			return 1;
		}
	}
	return 0;
}

void MainWindow::init(){
	setupUi( this );
	connect( chooseFileBtn, SIGNAL(clicked()), actionOpen, SLOT(trigger()) );
	connect( loadFileBtn, SIGNAL(clicked()), this, SLOT(loadClicked()) );

	connect( chooseLearningBtn, SIGNAL(clicked()), actionLoadLerningSet, SLOT(trigger()) );
	connect( loadLearningBtn, SIGNAL(clicked()), this, SLOT(loadLearningClicked()) );

	connect( audioSignalDraw->getAudioDraw(), SIGNAL(viewRegionChanged(int, int)), filteredDraw, SLOT(changeViewRegion(int, int)));
	connect( audioSignalDraw->getAudioDraw(), SIGNAL(viewRegionChanged(int, int)), energyDraw->getEnergy(), SLOT(changeViewRegion(int, int)));
	connect( filteredDraw->getAudioDraw(), SIGNAL(viewRegionChanged(int, int)), audioSignalDraw, SLOT(changeViewRegion(int, int)) );

	connect( audioSignalDraw->getAudioDraw(), SIGNAL(samplesSelected(int, int)), this, SLOT(selectedSamples(int, int)));
	connect( filteredDraw->getAudioDraw(), SIGNAL(samplesSelected(int, int)), this, SLOT(selectedSamples(int, int)) );

	connect( audioSignalDraw->getSlider(), SIGNAL(valueChanged(int)), filteredDraw->getSlider(), SLOT(setValue(int)) );
	connect( audioSignalDraw->getSlider(), SIGNAL(valueChanged(int)), segmentDraw->getSlider(), SLOT(setValue(int)) );

	connect( energyDraw->getEnergy(), SIGNAL(selectedFragments(list<sRegion>&)), audioSignalDraw->getAudioDraw(), SLOT(setSelection(list<sRegion>&)) );
	connect( energyDraw->getEnergy(), SIGNAL(selectedFragments(list<sRegion>&)), filteredDraw->getAudioDraw(), SLOT(setSelection(list<sRegion>&)) );

	connect( actionOpen, SIGNAL(triggered()), this, SLOT(chooseClicked()) );
	connect( actionLoadLerningSet, SIGNAL(triggered()), this, SLOT(chooseDirectoryClicked()) );
	connect( actionPlaySelection, SIGNAL(triggered()), this, SLOT(playSelected()) );
	connect( actionRecord, SIGNAL(triggered()), this, SLOT(record()) );
	connect( actionStop, SIGNAL(triggered()), this, SLOT(stopPlaying()) );
	connect( actionExit, SIGNAL(triggered()), this, SLOT(close()) );
	connect( actionMakeSpectrogram, SIGNAL(triggered()), audioSignalDraw->getAudioDraw(), SLOT(emitSelected()));
	connect( actionZoomIn, SIGNAL(triggered()), audioSignalDraw->getAudioDraw(), SLOT(zoomSelected()));
	connect( actionZoom11, SIGNAL(triggered()), audioSignalDraw->getAudioDraw(), SLOT(zoom11()));
	connect( actionZoomOut, SIGNAL(triggered()), audioSignalDraw->getAudioDraw(), SLOT(zoomOut()));
	connect( actionCompare, SIGNAL(triggered()), this, SLOT(openCompareWindow()) );
	connect( actionSettings, SIGNAL(triggered()), this, SLOT(openSettingsWindow()) );
	connect( actionSaveSelection, SIGNAL(triggered()), this, SLOT(saveSelection()) );
	connect( actionOpenLearningEditor, SIGNAL(triggered()), this, SLOT(openLearningEditor()) );
	connect( actionReadLearning, SIGNAL(triggered()), this, SLOT(readLearningFile()) );

	filteredDraw->getAudioDraw()->setSelectable(false);
	loadedFile = NULL;
	sample_tmp = NULL;
	audio_BufferSize = 8192;
	play_from = play_to = 0;
	playing = false;
	recording = false;
	try {
		audio = new RtAudio(0/*deviceId*/, 1/*channels*/, 0, 1, RTAUDIO_FLOAT64, 44100/*sampleRate*/, &audio_BufferSize, 4);
	}
	catch (RtError &error) {
		error.printMessage();
		exit(EXIT_FAILURE);
	}
	colorerList.push_back(new CTryColor);
	colorerList.push_back(new CColorWhite);
	colorerList.push_back(new CBlackWhiteColor);
	colorerList.push_back(new CWhiteBlackColor);
	colorsModel = new ColorListModel(colorerList, this);
	CSpectrogram::COLORER = colorerList[0];
}

MainWindow::~MainWindow(){
	audio->cancelStreamCallback();
	audio->abortStream();
	delete audio;
	delete colorsModel;
	if (sample_tmp != NULL){
		delete sample_tmp;
	}
	for (uint i=0; i<colorerList.size(); i++){
		delete colorerList[i];
	}
}

void MainWindow::chooseClicked(){
	QString s = QFileDialog::getOpenFileName(
			this,
			"Choose a file to open",
			"./",
			"Wave/MP3 (*.wav *.wave *.mp3)");
	if (s != ""){
		fileNameEdt->setText(s);
		loadClicked();
	}
}

void MainWindow::chooseDirectoryClicked(){
	QString s = QFileDialog::getExistingDirectory(
			this, "Choose a directory with learning set samples",
			"./", QFileDialog::ShowDirsOnly);
	if (s != ""){
		learningDirectoryEdt->setText(s);
		loadLearningClicked();
	}
}

extern CFilter MP3Filter;
void MainWindow::loadClicked(){
	stopPlaying();
	try{
#ifdef __DEBUG__
		printf("Loading started\n");
#endif
		const QString& filename = fileNameEdt->text();
		loadedFile = CFileFactory::createCFile(filename.toStdString());
		if (loadedFile == NULL){
			fprintf(stderr, "Unable to open file [NULL returned]\n");
			throw exception();
		}
		int size = loadedFile->framesLeft();
		size = min (size, 44100*60*5);
		samples.clear();
		audioSignalDraw->setSignal(samples);
		fSamples.clear();
		filteredDraw->setSignal(fSamples);
		energyDraw->getEnergy()->setSignal(fSamples);
		if (sample_tmp != NULL) {
			delete sample_tmp;
			sample_tmp = NULL;
		}

		for (int i=0; i<50; ++i){
			MP3Filter(0.0);
		}
		progressBar->setMinimum(0);
		progressBar->setMaximum(size);
		statusBar()->showMessage("Loading file...");
#ifdef __DEBUG__
		printf("Samples to load: %d\n", size);
#endif
		progressBar->setValue(0);
		if (size > 1){
			samples.resize(size);
			fSamples.resize(size);
			for (int i=0; i<size; i++){
				samples[i] = loadedFile->read();
				fSamples[i] = MP3Filter(samples[i]);
				if (i%(size/100) == 0){
					progressBar->setValue(i);
				}
			}
		} else {
			progressBar->setMaximum(-1);
			while (loadedFile->readPossible() && samples.size() < MAX_FRAMES){
				samples.push_back(loadedFile->read());
				fSamples.push_back(MP3Filter(samples.back()));
#ifdef __DEBUG__
				if (samples.size() % 44100 == 0){
					int tmp = samples.size();
					printf("\rLoaded: %d (%d:%02d:%02d)", tmp, tmp/44100/3600, (tmp/44100/60)%60, (tmp/44100)%60);
				}
#endif
			}
		}
		progressBar->setMaximum(1);
		progressBar->setValue(1);
		statusBar()->showMessage("");

		audioSignalDraw->setSignal(samples);
		filteredDraw->setSignal(fSamples);
		energyDraw->getEnergy()->setSignal(fSamples);
		segmentDraw->setSignal();
		spectrogram->setSample(NULL);
		normalizedSpectrogram->setSample(NULL);
		bestMatchSpectrogram->setSample(NULL);

		for (int i=0; i<50; ++i){
			MP3Filter(0.0);
		}

	} catch (exception e){
		QString msg = "File '";
		msg += fileNameEdt->text();
		msg += "' could not be opened";
		QMessageBox::information (this, "Can't open file", msg, QMessageBox::Ok);
	}
}

// void MainWindow::signalZoomY(int zoom){
// 	audioSignalDraw->setZoomY(zoom);
// 	filteredDraw->setZoomY(zoom);
// 	segmentDraw->setZoomY(zoom);
// }

void MainWindow::selectedCSample(CSample* sample){
	if (sample == NULL){
		spectrogram->setSample(sample, true);
		normalizedSpectrogram->setSample(sample, false);
		bestMatchSpectrogram->setSample(NULL, false);
		return;
	}
	audioSignalDraw->getAudioDraw()->setChosen(sample->getStartSampleNo(), sample->getEndSampleNo());
	filteredDraw->getAudioDraw()->setChosen(sample->getStartSampleNo(), sample->getEndSampleNo());
	selectedFrames.resize(sample->getFramesCount());
	double* frames = sample->getFrames();
	double maxV = 0;
	for (uint i=0; i<selectedFrames.size(); i++){
		selectedFrames[i] = frames[i];
		maxV = max(maxV, abs(selectedFrames[i]));
	}
	segmentDraw->setSignal(selectedFrames);
	spectrogram->setSample(sample, true);
	normalizedSpectrogram->setSample(sample, false);
	CSample * bestMatch = test(sample, learning);
	if (bestMatch != NULL){
		bestMatchSpectrogram->setSample(bestMatch, false);
		char buf[50];
		sprintf(buf, "%s (%3.2g)", bestMatch->getName().c_str(), sample->differ(*bestMatch));
		bestMatchSpectrogram->setDescription(buf);
	} else {
		bestMatchSpectrogram->setSample(NULL, false);
	}
}

void MainWindow::selectedSamples(int start, int end){
	if (start < 0){
		return;
	}
	// printf("Od: %d do: %d\n", start, end);
	try{
		sample_tmp = new CSample(fSamples, start, end-start, 44100, 0, start, end, 0);
		selectedCSample(sample_tmp);
	} catch(...){
		fprintf(stderr, "To short region selected\n");
	}
}

void MainWindow::loadLearningClicked(){
	for (vector<CSample*>::iterator it = learning.begin(); it != learning.end(); ++it){
		delete (*it);
		*it = NULL;
	}
	learning.clear();
	const QString& filename = learningDirectoryEdt->text();
	statusBar()->showMessage("Loading learning set, please wait...");
	progressBar->setMinimum(0);
	progressBar->setMaximum(100);
	progressBar->setValue(0);
	learning = readLearning(filename.toStdString().c_str(), progressBar);
	printf("Learning set size: %d\n", learning.size());
	statusBar()->showMessage("");
	progressBar->setMaximum(100);
	progressBar->setValue(100);
}

void MainWindow::playSelected(){
	stopPlaying();
	audio->cancelStreamCallback();
	if (fSamples.size() <= 0){
		return;
	}
	audio->setStreamCallback(MainWindow::playRecordCallback, (void*)&fSamples);
	// audio->setStreamCallback(playCallback, samples);
	sRegion sel = audioSignalDraw->getAudioDraw()->getSelection();
	if (sel.end == -1){
		sel = audioSignalDraw->getAudioDraw()->getViewRegion();
	}
	play_from = sel.start;
	play_to = sel.end;
	playing = true;
	// printf("Play from: %d to %d\n", play_from, play_to);
	try{
		audio->startStream();
	} catch (...) {
		fprintf(stderr, "Unable to start playback\n");
		if (QMessageBox::critical 
				(this, "Unable to start playback", "Unable to start playback. Should I try again?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes){
			playSelected();
		}
	}
}

void MainWindow::stopPlaying(){
	audio->abortStream();
	if (recording){
		recording = false;
		audioSignalDraw->setSignal(samples);
		filteredDraw->setSignal(fSamples);
		energyDraw->getEnergy()->setSignal(fSamples);
	} else if (playing){
		playing = false;
		audioSignalDraw->getAudioDraw()->setMarker(-1);
	}
}

void MainWindow::record(){
	if (samples.size() > 0 && QMessageBox::warning(this, "Current audio will be lost.", "Current audio will be deleted. Do you want to proceed?", QMessageBox::Ok |  QMessageBox::Default, QMessageBox::Cancel) != QMessageBox::Ok){
		return;
	}
	samples.clear();
	fSamples.clear();
	stopPlaying();
	recording = true;
	audioSignalDraw->setSignal(samples);
	filteredDraw->setSignal(fSamples);
	energyDraw->getEnergy()->setSignal(fSamples);
	audio->setStreamCallback(MainWindow::playRecordCallback, NULL);
	audio->startStream();
}

void MainWindow::openCompareWindow(){
	if (sample_tmp == NULL){
		QMessageBox::information (this, "Select signal...", "To open comparison window you have to make spectrogram of audio fragment first.", QMessageBox::Ok);
		return;
	}
	if (learning.size() == 0){
		if (QMessageBox::No == QMessageBox::information (this, "Load learning set...", "To open comparison window you have to load learning set. Do you want to do it now?", QMessageBox::Yes, QMessageBox::No)){
			return;
		}
		actionLoadLerningSet->trigger();
		if (learning.size() == 0){
			return;
		}
	}
	ComparisonWindow compWindow(this, sample_tmp, learning);
	compWindow.exec();
}

void MainWindow::openSettingsWindow(){
	SettingsDialog dial(this);
	dial.xScaleSB->setValue(CSpectrogram::X_SCALE);
	dial.brightnessSB->setValue(CSpectColor::BRIGHTNESS);
	dial.colorsCB->setModel(colorsModel);
	dial.colorsCB->setCurrentIndex(dial.colorsCB->findData((int)(CSpectrogram::COLORER), 9999));
	if (dial.exec() == QDialog::Accepted){
		CSpectrogram::X_SCALE = dial.xScaleSB->value();
		CSpectColor::BRIGHTNESS = dial.brightnessSB->value();
		QVariant q = dial.colorsCB->itemData(dial.colorsCB->currentIndex(), 9999);
		CSpectColor* colorer = (CSpectColor*)q.value<int>();
		CSpectrogram::COLORER = colorer;
		update();
	}
}

void MainWindow::saveSelection(){
	sRegion sel = audioSignalDraw->getAudioDraw()->getSelection();
	if (sel.start == -1 || samples.size() <= 0){
		return;
	}
	QString s	= QFileDialog::getSaveFileName ( this, "Save as...", "./", "Wave file (*.wav *.wave)");
	if (s != ""){
		SF_INFO sfinfo;
		sfinfo.samplerate = 44100;
		sfinfo.channels = 1;
		sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
		SNDFILE* file = sf_open(s.toStdString().c_str(), SFM_WRITE, &sfinfo);
		if (file == NULL){
			QMessageBox::critical(this, "Unable to create file.", "Unable to create file you choosed. Selection not saved!", QMessageBox::Ok |  QMessageBox::Default, QMessageBox::NoButton);
			return;
		}
		sf_write_double(file, &samples[sel.start], sel.end-sel.start+1);
		sf_close(file);
	}
}

void MainWindow::openLearningEditor(){
	if (learning.size() == 0){
		if (QMessageBox::No == QMessageBox::information (this, "Load learning set...", "To open comparison window you have to load learning set. Do you want to do it now?", QMessageBox::Yes, QMessageBox::No)){
			return;
		}
		actionLoadLerningSet->trigger();
		if (learning.size() == 0){
			return;
		}
	}
	LearningDialog dial(this);
	LearningListModel llmodel(learning, this);
	dial.learningList->setModel(&llmodel);
	dial.connectAll();
	if (dial.exec() == QDialog::Accepted){
		printf("Saving...\n");
		learning = llmodel.getLearning();
		saveSamplesToFile(learning, "saved.freq");
		selectedCSample(sample_tmp);
		update();
	}

}

void MainWindow::readLearningFile(){
	QString s = QFileDialog::getOpenFileName(
			this,
			"Choose a file to open",
			"./",
			"Learning set (*.freq)");
	if (s != ""){
		learning = readLearningFromFile(s.toStdString().c_str());
#ifdef __DEBUG__
		printf("Read %d samples\n", learning.size());
#endif
		selectedCSample(sample_tmp);
	}
}

int main(int argc, char *argv[]) {
	if (argc > 1){
		main_detect(argc, argv);
		return 0;
	}
	QApplication app(argc, argv);
	MainWindow window;
	myWindow = &window;
	window.show();
	return app.exec();
}
