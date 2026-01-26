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
#include <QPushButton>
#include <QAction>
#include <QSlider>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QResource>

#include <algorithm>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>

using namespace std;
QMutex samplesMutex;

uint play_from, play_to;

namespace {
bool bscDebugEnabled() {
	static int enabled = -1;
	if (enabled < 0) {
		const char* env = std::getenv("BSC_DEBUG");
		enabled = (env && *env) ? 1 : 0;
	}
	return enabled == 1;
}

void bscDebugLog(const char* fmt, ...) {
	if (!bscDebugEnabled()) {
		return;
	}
	va_list args;
	va_start(args, fmt);
	std::fprintf(stderr, "BSC: ");
	std::vfprintf(stderr, fmt, args);
	std::fprintf(stderr, "\n");
	va_end(args);
}
} // namespace

static vector<CSample*> toRawSamples(const vector<unique_ptr<CSample>>& samples){
	vector<CSample*> raw;
	raw.reserve(samples.size());
	for (const auto& sample : samples){
		raw.push_back(sample.get());
	}
	return raw;
}

const uint MAX_FRAMES = 44100/*fps*/ * 60 /*seconds*/ * 5 /*minutes*/;

int MainWindow::playRecordCallback(void *outputBuffer,
                                   void *inputBuffer,
                                   unsigned int nBufferFrames,
                                   double /*streamTime*/,
                                   RtAudioStreamStatus /*status*/,
                                   void *userData){
	MainWindow* that = static_cast<MainWindow*>(userData);
	if (that == NULL){
		return 0;
	}
	AudioDraw* asd = that->audioSignalDraw->getAudioDraw();
	double * out = static_cast<double *>(outputBuffer);
	const double * in = static_cast<const double *>(inputBuffer);
	if (that->playing){
		if (play_from >= play_to){
			if (out != NULL){
				std::fill(out, out + nBufferFrames, 0.0);
			}
			asd->setMarker(-1);
			return 1;
		}
		unsigned int remaining = play_to - play_from;
		unsigned int count = std::min(nBufferFrames, remaining);
		if (out != NULL){
			for (unsigned int i = 0; i < count; ++i){
				out[i] = that->samples[play_from++];
			}
			if (count < nBufferFrames){
				std::fill(out + count, out + nBufferFrames, 0.0);
			}
		} else {
			play_from += count;
		}
		asd->setMarker(play_from);
	} else if (that->recording){
		if (out != NULL){
			std::fill(out, out + nBufferFrames, 0.0);
		}
		if (in != NULL){
			samplesMutex.lock();
			for (unsigned int i = 0; i < nBufferFrames; ++i){
				that->samples.push_back(in[i]);
				that->fSamples.push_back(MP3Filter(in[i]));
			}
			samplesMutex.unlock();
		}
		that->audioSignalDraw->setSignal(that->samples);
		that->filteredDraw->setSignal(that->fSamples);
		// that->energyDraw->getEnergy()->updateSignal(that->fSamples);
		if (that->samples.size() > 44100 * 60){
			return 1;
		}
	} else if (out != NULL){
		std::fill(out, out + nBufferFrames, 0.0);
	}
	return 0;
}

void MainWindow::init(){
	setupUi( this );
	connect(chooseFileBtn, &QPushButton::clicked, actionOpen, &QAction::trigger);
	connect(loadFileBtn, &QPushButton::clicked, this, &MainWindow::loadClicked);

	connect(chooseLearningBtn, &QPushButton::clicked, actionLoadLerningSet, &QAction::trigger);
	connect(loadLearningBtn, &QPushButton::clicked, this, &MainWindow::loadLearningClicked);

	connect(audioSignalDraw->getAudioDraw(), &AudioDraw::viewRegionChanged, filteredDraw, &SignalDraw::changeViewRegion);
	connect(audioSignalDraw->getAudioDraw(), &AudioDraw::viewRegionChanged, energyDraw->getEnergy(), &CEnergyDraw::changeViewRegion);
	connect(filteredDraw->getAudioDraw(), &AudioDraw::viewRegionChanged, audioSignalDraw, &SignalDraw::changeViewRegion);

	connect(audioSignalDraw->getAudioDraw(), &AudioDraw::samplesSelected, this, &MainWindow::selectedSamples);
	connect(filteredDraw->getAudioDraw(), &AudioDraw::samplesSelected, this, &MainWindow::selectedSamples);

	connect(audioSignalDraw->getSlider(), &QSlider::valueChanged, filteredDraw->getSlider(), &QSlider::setValue);
	connect(audioSignalDraw->getSlider(), &QSlider::valueChanged, segmentDraw->getSlider(), &QSlider::setValue);

	connect(energyDraw->getEnergy(), &CEnergyDraw::selectedFragments, audioSignalDraw->getAudioDraw(), &AudioDraw::setSelection);
	connect(energyDraw->getEnergy(), &CEnergyDraw::selectedFragments, filteredDraw->getAudioDraw(), &AudioDraw::setSelection);

	connect(actionOpen, &QAction::triggered, this, &MainWindow::chooseClicked);
	connect(actionLoadLerningSet, &QAction::triggered, this, &MainWindow::chooseDirectoryClicked);
	connect(actionPlaySelection, &QAction::triggered, this, &MainWindow::playSelected);
	connect(actionRecord, &QAction::triggered, this, &MainWindow::record);
	connect(actionStop, &QAction::triggered, this, &MainWindow::stopPlaying);
	connect(actionExit, &QAction::triggered, this, &MainWindow::close);
	connect(actionMakeSpectrogram, &QAction::triggered, audioSignalDraw->getAudioDraw(), &AudioDraw::emitSelected);
	connect(actionZoomIn, &QAction::triggered, audioSignalDraw->getAudioDraw(), &AudioDraw::zoomSelected);
	connect(actionZoom11, &QAction::triggered, audioSignalDraw->getAudioDraw(), &AudioDraw::zoom11);
	connect(actionZoomOut, &QAction::triggered, audioSignalDraw->getAudioDraw(), &AudioDraw::zoomOut);
	connect(actionCompare, &QAction::triggered, this, &MainWindow::openCompareWindow);
	connect(actionSettings, &QAction::triggered, this, &MainWindow::openSettingsWindow);
	connect(actionSaveSelection, &QAction::triggered, this, &MainWindow::saveSelection);
	connect(actionOpenLearningEditor, &QAction::triggered, this, &MainWindow::openLearningEditor);
	connect(actionReadLearning, &QAction::triggered, this, &MainWindow::readLearningFile);

	filteredDraw->getAudioDraw()->setSelectable(false);
	loadedFile.reset();
	sample_tmp.reset();
	audio_BufferSize = 8192;
	play_from = play_to = 0;
	playing = false;
	recording = false;
	try {
		audio = make_unique<RtAudio>();
		RtAudio::StreamParameters outputParams;
		outputParams.deviceId = audio->getDefaultOutputDevice();
		outputParams.nChannels = 1;
		outputParams.firstChannel = 0;
		RtAudio::StreamParameters inputParams;
		inputParams.deviceId = audio->getDefaultInputDevice();
		inputParams.nChannels = 1;
		inputParams.firstChannel = 0;
		unsigned int bufferFrames = audio_BufferSize;
		audio->openStream(&outputParams,
		                  &inputParams,
		                  RTAUDIO_FLOAT64,
		                  44100,
		                  &bufferFrames,
		                  &MainWindow::playRecordCallback,
		                  this);
		audio_BufferSize = bufferFrames;
	}
	catch (const RtAudioError &error) {
		fprintf(stderr, "RtAudio error: %s\n", error.what());
		exit(EXIT_FAILURE);
	}
	colorerList.push_back(make_unique<CTryColor>());
	colorerList.push_back(make_unique<CColorWhite>());
	colorerList.push_back(make_unique<CBlackWhiteColor>());
	colorerList.push_back(make_unique<CWhiteBlackColor>());
	colorsModel = make_unique<ColorListModel>(colorerList, nullptr);
	CSpectrogram::COLORER = colorerList[0].get();
}

MainWindow::~MainWindow(){
	if (audio){
		try {
			if (audio->isStreamRunning()){
				audio->abortStream();
			}
			if (audio->isStreamOpen()){
				audio->closeStream();
			}
		} catch (const RtAudioError &error) {
			fprintf(stderr, "RtAudio shutdown error: %s\n", error.what());
		}
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
		QElapsedTimer timer;
		timer.start();
		bscDebugLog("GUI load: starting.");
#ifdef __DEBUG__
		printf("Loading started\n");
#endif
		const QString& filename = fileNameEdt->text();
		bscDebugLog("GUI load: opening '%s'.", filename.toStdString().c_str());
		loadedFile = CFileFactory::createCFile(filename.toStdString());
		if (!loadedFile){
			fprintf(stderr, "Unable to open file [NULL returned]\n");
			throw exception();
		}
		int size = loadedFile->framesLeft();
		bscDebugLog("GUI load: frames=%d, sampleRate=%u.", size, loadedFile->getSampleRate());
		size = min (size, 44100*60*5);
		samples.clear();
		audioSignalDraw->setSignal(samples);
		fSamples.clear();
		filteredDraw->setSignal(fSamples);
		energyDraw->getEnergy()->setSignal(fSamples);
		sample_tmp.reset();

		for (int i=0; i<50; ++i){
			MP3Filter(0.0);
		}
		progressBar->setMinimum(0);
		progressBar->setMaximum(size);
		statusBar()->showMessage("Loading file...");
		QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
#ifdef __DEBUG__
		printf("Samples to load: %d\n", size);
#endif
		progressBar->setValue(0);
		if (size > 1){
			samples.resize(size);
			fSamples.resize(size);
			const int updateStep = max(1, size / 100);
			for (int i=0; i<size; i++){
				samples[i] = loadedFile->read();
				fSamples[i] = MP3Filter(samples[i]);
				if (i % updateStep == 0){
					progressBar->setValue(i);
					QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
				}
			}
		} else {
			progressBar->setMaximum(-1);
			while (loadedFile->readPossible() && samples.size() < MAX_FRAMES){
				samples.push_back(loadedFile->read());
				fSamples.push_back(MP3Filter(samples.back()));
				if (samples.size() % 4096 == 0) {
					QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
				}
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
		QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
		bscDebugLog("GUI load: decoded %zu samples in %lld ms.", samples.size(), timer.elapsed());

		QElapsedTimer stageTimer;
		stageTimer.start();
		bscDebugLog("GUI load: rendering waveform.");
		audioSignalDraw->setSignal(samples);
		bscDebugLog("GUI load: waveform rendered in %lld ms.", stageTimer.elapsed());
		stageTimer.restart();
		bscDebugLog("GUI load: rendering filtered waveform.");
		filteredDraw->setSignal(fSamples);
		bscDebugLog("GUI load: filtered waveform rendered in %lld ms.", stageTimer.elapsed());
		stageTimer.restart();
		bscDebugLog("GUI load: rendering energy plot.");
		energyDraw->getEnergy()->setSignal(fSamples);
		bscDebugLog("GUI load: energy plot rendered in %lld ms.", stageTimer.elapsed());
		stageTimer.restart();
		bscDebugLog("GUI load: rendering segments.");
		segmentDraw->setSignal();
		bscDebugLog("GUI load: segments rendered in %lld ms.", stageTimer.elapsed());
		spectrogram->setSample(NULL);
		normalizedSpectrogram->setSample(NULL);
		bestMatchSpectrogram->setSample(NULL);

		for (int i=0; i<50; ++i){
			MP3Filter(0.0);
		}
		bscDebugLog("GUI load: finished.");

	} catch (const exception& e){
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
	const std::vector<double>& frames = sample->getFrames();
	double maxV = 0;
	for (uint i=0; i<selectedFrames.size(); i++){
		selectedFrames[i] = frames[i];
		maxV = max(maxV, abs(selectedFrames[i]));
	}
	segmentDraw->setSignal(selectedFrames);
	spectrogram->setSample(sample, true);
	normalizedSpectrogram->setSample(sample, false);
	auto learningRaw = toRawSamples(learning);
	CSample * bestMatch = test(sample, learningRaw);
	if (bestMatch != NULL){
		bestMatchSpectrogram->setSample(bestMatch, false);
		QString desc = QString("%1 (%2)").arg(bestMatch->getName().c_str()).arg(sample->differ(*bestMatch), 0, 'g', 3);
		bestMatchSpectrogram->setDescription(desc.toStdString().c_str());
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
		sample_tmp = make_unique<CSample>(fSamples, start, end-start, 44100, 0, start, end, 0, &fft);
		selectedCSample(sample_tmp.get());
	} catch(...){
		fprintf(stderr, "To short region selected\n");
	}
}

void MainWindow::loadLearningClicked(){
	learning.clear();
	const QString& filename = learningDirectoryEdt->text();
	statusBar()->showMessage("Loading learning set, please wait...");
	progressBar->setMinimum(0);
	progressBar->setMaximum(100);
	progressBar->setValue(0);
	learning = readLearning(filename.toStdString().c_str(), manager, progressBar);
	printf("Learning set size: %d\n", (int)learning.size());
	statusBar()->showMessage("");
	progressBar->setMaximum(100);
	progressBar->setValue(100);
}

void MainWindow::playSelected(){
	stopPlaying();
	if (fSamples.size() <= 0){
		return;
	}
	sRegion sel = audioSignalDraw->getAudioDraw()->getSelection();
	if (sel.end == -1){
		sel = audioSignalDraw->getAudioDraw()->getViewRegion();
	}
	const int start = max(0, sel.start);
	const int end = min<int>(sel.end, samples.size());
	if (end <= start){
		return;
	}
	play_from = static_cast<uint>(start);
	play_to = static_cast<uint>(end);
	playing = true;
	// printf("Play from: %d to %d\n", play_from, play_to);
	try{
		audio->startStream();
	} catch (const RtAudioError &error) {
		fprintf(stderr, "Unable to start playback: %s\n", error.what());
		if (QMessageBox::critical 
				(this, "Unable to start playback", "Unable to start playback. Should I try again?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes){
			playSelected();
		}
	}
}

void MainWindow::stopPlaying(){
	if (audio && audio->isStreamRunning()){
		audio->abortStream();
	}
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
	try{
		audio->startStream();
	} catch (const RtAudioError &error) {
		fprintf(stderr, "Unable to start recording: %s\n", error.what());
	}
}

void MainWindow::openCompareWindow(){
	if (!sample_tmp){
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
	auto learningRaw = toRawSamples(learning);
	ComparisonWindow compWindow(this, sample_tmp.get(), learningRaw);
	compWindow.exec();
}

void MainWindow::openSettingsWindow(){
	SettingsDialog dial(this);
	dial.xScaleSB->setValue(CSpectrogram::X_SCALE);
	dial.brightnessSB->setValue(CSpectColor::BRIGHTNESS);
	dial.colorsCB->setModel(colorsModel.get());
	dial.colorsCB->setCurrentIndex(dial.colorsCB->findData(QVariant::fromValue(reinterpret_cast<quintptr>(CSpectrogram::COLORER)), 9999));
	if (dial.exec() == QDialog::Accepted){
		CSpectrogram::X_SCALE = dial.xScaleSB->value();
		CSpectColor::BRIGHTNESS = dial.brightnessSB->value();
		QVariant q = dial.colorsCB->itemData(dial.colorsCB->currentIndex(), 9999);
		CSpectColor* colorer = reinterpret_cast<CSpectColor*>(q.value<quintptr>());
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
		auto learningRaw = toRawSamples(learning);
		saveSamplesToFile(learningRaw, "saved.freq");
		selectedCSample(sample_tmp.get());
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
		selectedCSample(sample_tmp.get());
	}
}

int main(int argc, char *argv[]) {
	if (argc > 1){
		main_detect(argc, argv);
		return 0;
	}
	QApplication app(argc, argv);
	Q_INIT_RESOURCE(Images);
	MainWindow window;
	window.show();
	return app.exec();
}
