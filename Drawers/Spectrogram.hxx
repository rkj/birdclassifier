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

#ifndef _SPECTROGRAM_HXX
#define _SPECTROGRAM_HXX

#include <QLabel>
#include <QGroupBox>
#include <QPaintEvent>
#include <list>
#include <cmath>
#include "detect/Audio.hxx"

// Note: Do not use "using namespace std" in headers
// Use std:: prefix explicitly to avoid namespace pollution

class CSpectColor {
	public:
		virtual QColor valueToColor(double value) = 0;
		virtual QColor valueToColorLog(double value, double min, double max) = 0;
		virtual QString description() = 0;
		virtual ~CSpectColor() {}
		static double BRIGHTNESS;
};

class CBlackWhiteColor : public CSpectColor {
	public:
		QString description(){
			return "Black & White";
		}
		QColor valueToColor(double value){
			int v = (int) (255*value);
			return QColor(v, v, v);
		}
		QColor valueToColorLog(double value, double min, double max){
			value -= min - 1;
			max -= min - 1;
			int v = (int)(255*log(value)/log(BRIGHTNESS));
			if (v > 255){
				// fprintf(stderr, "Value2>255: %g, Val: %g, max: %g, min: %g, v: %d\n", value, val, max, min, v);
				v = 255;
			} else if (v < 0) {
				// fprintf(stderr, "Value2<0: %g, max: %g, min: %g, v: %d\n",  value, max, min, v);
				v = 0;
			}
			return QColor(v, v, v);

		}
};

class CWhiteBlackColor : public CSpectColor {
	public:
		QString description(){
			return "White & Black";
		}
		QColor valueToColor(double value){
			int v = 255 - (int) (255*value);
			return QColor(v, v, v);
		}
		QColor valueToColorLog(double value, double min, double max){
			value -= min - 1;
			max -= min - 1;
			int v = 255 - (int)(255*log(value)/log(BRIGHTNESS));
			if (v > 255){
				// fprintf(stderr, "Value2>255: %g, Val: %g, max: %g, min: %g, v: %d\n", value, val, max, min, v);
				v = 255;
			} else if (v < 0) {
				// fprintf(stderr, "Value2<0: %g, max: %g, min: %g, v: %d\n",  value, max, min, v);
				v = 0;
			}
			return QColor(v, v, v);

		}
};

class CTryColor : public CSpectColor {
	public:
		QString description(){
			return "Color Black";
		}
		QColor valueToColor(double value){
			const double CUT = 0.25;
			if (value > CUT){
				return QColor::fromHsv((int)((1.0-value)*220/(1.0-CUT)), 255, 255);
			} else {
				return QColor::fromHsv((int)((CUT-value)/CUT*20+220), 255, (int)(value/CUT*255));
			}
		}
		QColor valueToColorLog(double value, double min, double max){
			value -= min - 1;
			max -= min - 1;
			double tmp = log(value);
			// const double cLog = log(1.250);
			const double cLog = log(BRIGHTNESS);
			double mLog = log(max);
			if (tmp > cLog){
				return QColor::fromHsv((int)((mLog-tmp)*220/(mLog-cLog)), 255, 255);
			} else {
				return QColor::fromHsv((int)((cLog-tmp)/cLog*20+220), 255, (int)(tmp/cLog*255));
			}
		}
};

class CColorWhite : public CSpectColor{
	public:
		QString description(){
			return "Color White";
		}
		QColor valueToColor(double value){
			const double CUT = 0.25;
			if (value > CUT){
				return QColor::fromHsv((int)((1.0-value)*220/(1.0-CUT)), 255, 255);
			} else {
				return QColor::fromHsv((int)((CUT-value)/CUT*20+220), (int)(value/CUT*255), 255);
			}
		}
		QColor valueToColorLog(double value, double min, double max){
			value -= min - 1;
			max -= min - 1;
			double tmp = log(value);
			// const double cLog = log(1.250);
			const double cLog = log(BRIGHTNESS);
			double mLog = log(max);
			if (tmp > cLog){
				return QColor::fromHsv((int)((mLog-tmp)*220/(mLog-cLog)), 255, 255);
			} else {
				return QColor::fromHsv((int)((cLog-tmp)/cLog*20+220), (int)(tmp/cLog*255), 255);
			}
		}
};

class CSpectrogram : public QLabel {
	Q_OBJECT
	public:
		static const int LEGEND_WIDTH = 60;
		static const int LEGEND_HEIGHT = 20;
		static CSpectColor* COLORER;
		static int X_SCALE;
		void setSample(CSample *, bool = true);
		int MaxFreq(){
			return maxFreq;
		}
		void setDescription(const QString&);

		CSpectrogram(){
			init();
		}
		CSpectrogram(QWidget* &gb) : QLabel(gb){
			init();
		}
		CSpectrogram(QGroupBox* &gb) : QLabel(gb){
			init();
		}
		~CSpectrogram(){
		}
	protected:
		// void mousePressEvent(QMouseEvent *event);
		// void mouseMoveEvent(QMouseEvent *event);
		// void mouseReleaseEvent(QMouseEvent *event);
		void paintEvent(QPaintEvent *event);
		//		void resizeEvent(QResizeEvent *event);

	private:
		void init();
		int maxFreq;
		SFrequencies * frequencies;
		OrigFrequencies * origFrequencies;
		double maxValue;
		double minValue;
		uint freqCount;
		QString description;
		int xStart;
		int yStart;
		int toPanelX(int x){
			return width()*x/freqCount;
		};
		int fromPanelX(int x){
			return freqCount*x/width();
		};
		int toPanelY(int y){
			// return height()*y/(FFT_SIZE/2);
			return height()-height()*y/(FFT_SIZE/2);
		};
		int fromPanelY(int y){
			// return (FFT_SIZE/2)*(y)/height();
			return (FFT_SIZE/2)*(height()-y)/height();
		};
		protected slots:
			void xStartChange(int _x){
				if (_x<0){
					return;
				}
				// printf("[%s] Ustawiam xStart na: %d\n", objectName().toStdString().c_str(), _x);
				xStart = _x;
				update();
			};
		void yStartChange(int _y){
			if (_y<0){
				return ;
			}
			// printf("[%s] Ustawiam yStart na: %d\n", objectName().toStdString().c_str(), _y);
			yStart = _y;
			update();
		};
signals:
};

#endif
