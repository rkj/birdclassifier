/*
	QTDetection, bird voice visualization and comparison.
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

#include "SpectroDraw.hxx"
#include <QScrollBar>
#include <algorithm>

void SpectroDraw::init(){
	setupUi(this);
	connect(horizontalScrollBar, &QScrollBar::valueChanged, spectrogram, &CSpectrogram::xStartChange);
	connect(verticalScrollBar, &QScrollBar::valueChanged, spectrogram, &CSpectrogram::yStartChange);
	tuneSliders();
	freqCount = 0;
}

SpectroDraw::SpectroDraw(QWidget* q) : QWidget(q){
	init();
}

SpectroDraw::SpectroDraw(QGroupBox* q) : QWidget(q){
	init();
}

void SpectroDraw::tuneSliders(){
	// horizontalScrollBar->setMaximum(std::max(freqCount - (spectrogram->width()-CSpectrogram::LEGEND_WIDTH)/CSpectrogram::X_SCALE, 0));
	horizontalScrollBar->setMaximum(std::max(freqCount - (spectrogram->width()-CSpectrogram::LEGEND_WIDTH)/CSpectrogram::X_SCALE, 0));
	horizontalScrollBar->setPageStep((spectrogram->width()-CSpectrogram::LEGEND_WIDTH)/CSpectrogram::X_SCALE);

	int left = std::max(spectrogram->MaxFreq() + CSpectrogram::LEGEND_HEIGHT + 15 - spectrogram->height(), 0);
	verticalScrollBar->setMaximum(left);
	verticalScrollBar->setPageStep(spectrogram->height());
	spectrogram->update();
}

void SpectroDraw::resizeEvent(QResizeEvent *){
	tuneSliders();
}

void SpectroDraw::setSample(CSample * sample, bool original){
	spectrogram->setSample(sample, original);
	this->original = original;
	if (sample != NULL){
		freqCount = sample->getFreqCount();
		tuneSliders();
	} else {
		freqCount = 0;
		horizontalScrollBar->setMaximum(0);
		verticalScrollBar->setMaximum(0);
	}
}

