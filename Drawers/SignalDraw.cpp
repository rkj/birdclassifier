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

#include "SignalDraw.hxx"
#include <QPainter>
#include <QScrollBar>
#include <QSlider>

SignalDraw::SignalDraw(QWidget* parent) : QWidget (parent), Ui::SignalDraw(){
	init();
}

void SignalDraw::init(){
	setupUi(this);

	connect(drawingArea, &AudioDraw::viewRegionChanged, this, &SignalDraw::changeViewRegion);
	connect(positionSB, &QScrollBar::valueChanged, drawingArea, &AudioDraw::changeViewRegionStart);
	connect(verticalSlider, &QSlider::valueChanged, drawingArea, &AudioDraw::setZoomY);
	verticalSlider->setValue(200);

	samples = 0;
	vrEnd = -1;
	vrStart = -1;
}

void SignalDraw::changeViewRegion(int start, int end){
	// printf("In: %s: %d %d\n", objectName().toStdString().c_str(), start, end);
	if (samples <= 0 || (vrStart == start && vrEnd == end)){
		return;
	}
	vrStart = start;
	vrEnd = end;

	drawingArea->changeViewRegion(start, end);
	positionSB->setMinimum(0);
	positionSB->setMaximum(samples - (end-start));
	positionSB->setValue(start);
	positionSB->setSingleStep((end-start)/drawingArea->width());
	positionSB->setPageStep(end-start);
	// printf("Out: %s: %d %d\n", objectName().toStdString().c_str(), start, end);
}

