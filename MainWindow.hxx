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

#ifndef _MAINWINDOW_HXX
#define _MAINWINDOW_HXX

#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QMessageBox>

#include <vector>
#include <memory>
#include <exception>

#include "detect/Files.hxx"
#include "detect/Filter.hxx"
#include "detect/Manager.hxx"
#include "SettingsDialog.hxx"
#include "ComparisonWindow.hxx"
#include <RtAudio.h>

extern bool playing;
extern uint play_from, play_to;
extern int playCallback(char*, int, void*);

class MainWindow : public QMainWindow, private Ui::MainWindow {
	Q_OBJECT

	public:
		MainWindow() : QMainWindow(){
			init();
		}
		~MainWindow();
	protected:

	private:
		std::vector<double> samples;
		std::vector<double> fSamples;
		unsigned int audio_BufferSize;

	private slots:
		void chooseClicked();
		void loadClicked();
		void chooseDirectoryClicked();
		void loadLearningClicked();
		// void signalZoomY(int zoom);
		void selectedSamples(int, int);
		void playSelected();
		void record();
		void stopPlaying();
		void openCompareWindow();
		void openLearningEditor();
		void openSettingsWindow();
		void saveSelection();
		void readLearningFile();

	private:
		std::unique_ptr<CSample> sample_tmp;
		std::unique_ptr<CFile> loadedFile;
		std::unique_ptr<RtAudio> audio;
		std::vector<std::unique_ptr<CSample>> learning;
		std::vector<double> selectedFrames;
		std::vector<std::unique_ptr<CSpectColor>> colorerList;
		std::unique_ptr<ColorListModel> colorsModel;
		void selectedCSample(CSample*);
		void init();
		bool recording;
		bool playing;
		static int playRecordCallback(void *outputBuffer,
		                              void *inputBuffer,
		                              unsigned int nBufferFrames,
		                              double streamTime,
		                              RtAudioStreamStatus status,
		                              void *userData);
};

#endif // _MAINWINDOW_HXX
