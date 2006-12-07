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

#ifndef _SIGNALDRAW_HXX
#define _SIGNALDRAW_HXX

#include "ui_SignalDraw.h"
#include <QtDesigner/QDesignerExportWidget>

class SignalDraw : public QWidget, private Ui::SignalDraw {
	Q_OBJECT
	public:
		SignalDraw(QWidget *parent = 0);

		void setSignal(){
			samples = 0;
			drawingArea->setSignal();
		}
		void setSignal(vector<double> &values){
			samples = values.size();
			drawingArea->setSignal(values);
		}
		AudioDraw* getAudioDraw() const {
			return drawingArea;
		}
		QSlider* getSlider() const {
			return verticalSlider;
		}
	protected:
		// void paintEvent(QPaintEvent *event);
	private:
		void init();
		uint samples;
		int vrStart, vrEnd;

	public slots:
		void changeViewRegion(int start, int end);
	signals:
};

#endif// _SIGNALDRAW_HXX
