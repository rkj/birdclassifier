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

#include "Spectrogram.hxx"
#include <QPainter>
#include <algorithm>

int CSpectrogram::X_SCALE = 2;
double CSpectColor::BRIGHTNESS = 1.0100;
CSpectColor* CSpectrogram::COLORER = NULL;
// void CSpectrogram::mousePressEvent(QMouseEvent *event){
// }
// void CSpectrogram::mouseMoveEvent(QMouseEvent *event){
// }
// void CSpectrogram::mouseReleaseEvent(QMouseEvent *event){
// }
void CSpectrogram::paintEvent(QPaintEvent *event){
	int xFrom = event->rect().x();
	int yFrom = event->rect().y();
	int xTo = (event->rect().width() - LEGEND_WIDTH)/X_SCALE + xFrom ;
	xTo = std::min((int)freqCount-xStart-1, xTo);
	int yTo = event->rect().height() + yFrom;

	QPainter paint(this);
	paint.setBrush(Qt::NoBrush);
	paint.setPen(Qt::red);

	if (origFrequencies != NULL){
		yTo = std::min(maxFreq - yStart, yTo);
		for (int x = xFrom; x < xTo; x++){
			for (int y = yFrom; y < yTo; y++){
				double value = origFrequencies[x+xStart].freq[maxFreq-y-yStart];
				// printf("{%5d}[%5d]: %f (Min: %f, max: %f)\n", fromPanelX(x), fromPanelY(y)*22100/128, minValue, maxValue);
				paint.setPen(COLORER->valueToColorLog(value, minValue, maxValue));
				for (int i=0; i<X_SCALE; i++){
					paint.drawPoint(LEGEND_WIDTH + X_SCALE*x+i, y);
				}
			}
		}

		//vertical legend
		paint.setPen(Qt::black);
		paint.drawLine(LEGEND_WIDTH-2, 0, LEGEND_WIDTH-2, maxFreq - yStart - 1);
		for (uint i=0; i<=maxFreq*44100/FFT_SIZE/2000; i++){
			int y = maxFreq - FFT_SIZE * i * 2000 / 44100 - 1;
			paint.drawLine(LEGEND_WIDTH-2, y - yStart, LEGEND_WIDTH-12, y - yStart);
			char buf[32];
			sprintf(buf, "%d", i*2000);
			paint.drawText(2, y-7 - yStart, 40, 20, Qt::AlignRight, buf);
		}
	} else if (frequencies != NULL) {
		yTo = std::min((int)(COUNT_FREQ-1-yStart), yTo);
		// printf("FREQ_COUNT: %d, width(): %d, height(): %d\n", freqCount, width(), height());
		for (int x = xFrom; x < xTo; x++){
			for (int y = yFrom; y < yTo; y++){
				double value = frequencies[x+xStart].freq[maxFreq-y-yStart];
				// printf("{%5d}[%5d]: %f (Min: %f, max: %f)\n", fromPanelX(x), fromPanelY(y)*22100/128, minValue, maxValue);
				if (value > 1.0){
					fprintf(stderr, "Value1>255: %g, %d, %d, %d %d\n", value, x, y, fromPanelX(x), (fromPanelY(y)-FIRST_FREQ)*44100/FFT_SIZE);
					value = 1.0;
				} else if (value < 0.0){
					fprintf(stderr, "Value1<0: %g, X: %d, xStart: %d, Y: %d, yStart: %d, Freq: %d, freqCount: %d, width(): %d\n", value, x, xStart, y, yStart, (FFT_SIZE/2-y-yStart)*44100/FFT_SIZE, freqCount, width());
					value = 0.0;
				}
				paint.setPen(COLORER->valueToColor(value));
				for (int i=0; i<X_SCALE; i++){
					paint.drawPoint(LEGEND_WIDTH + X_SCALE*x+i, y);
				}
			}
		}

		//vertical legend
		paint.setPen(Qt::black);
		paint.drawLine(LEGEND_WIDTH-2, 0, LEGEND_WIDTH-2, maxFreq - yStart - 1);
		for (uint i=0; i<=maxFreq*44100/FFT_SIZE/2000; i++){
			int y = maxFreq - FFT_SIZE * i * 2000 / 44100 - 1;
			paint.drawLine(LEGEND_WIDTH-2, y - yStart, LEGEND_WIDTH-12, y - yStart);
			char buf[32];
			sprintf(buf, "%d", i*2000+FIRST_FREQ*44100/FFT_SIZE);
			paint.drawText(0, y-7 - yStart, LEGEND_WIDTH-13, 20, Qt::AlignRight, buf);
		}
	} else {
		return;
	}
	//Legenda pozioma
	paint.setPen(Qt::black);
	paint.drawText(LEGEND_WIDTH, maxFreq + LEGEND_HEIGHT + 12 - yStart, description);
	const int STEP = 40;
	int to = std::min(width(), (int)freqCount*X_SCALE+LEGEND_WIDTH);
	int start = LEGEND_WIDTH-xStart%STEP;
	if (start < LEGEND_WIDTH){
		start += STEP;
	}
	for (int i=start; i<to; i+=STEP){
		paint.drawLine(i, maxFreq + 1 - yStart, i, maxFreq + LEGEND_HEIGHT - 12 - yStart);
		int frame = (xStart+i-LEGEND_WIDTH) * SEGMENT_FRAMES / X_SCALE;
		char buf[10];
		sprintf(buf, "%.2f", 1.0*frame/44100);
		paint.drawText(i-STEP/2+1, maxFreq + LEGEND_HEIGHT - 10 - yStart, STEP-1, 10, Qt::AlignCenter, buf);
	}
	paint.drawLine(LEGEND_WIDTH, maxFreq + 1 - yStart, to, maxFreq + 1 - yStart);

	QColor color = paint.background().color();
	color.setAlpha(150);
	paint.setBrush(QBrush(color));
	paint.setPen(Qt::NoPen);
	paint.drawRect(0, 0, 25, 14);
	paint.setPen(Qt::red);
	paint.drawText(0, 0, 25, 15, Qt::AlignLeft | Qt::AlignTop, "[Hz]");
}

void CSpectrogram::setSample(CSample * sample, bool original){
	if (sample == NULL){
		description = "";
		frequencies = NULL;
		origFrequencies = NULL;
		maxFreq = 0;
		update();
		return;
	}
	freqCount = sample->getFreqCount();
	setDescription(sample->getName().c_str());
	if (original){
		maxFreq = FFT_SIZE * 15000 / 44100;
		minValue = 10e10;
		maxValue = -10e10;
		// printf("%g; %g\n", minValue, maxValue);
		origFrequencies = const_cast<OrigFrequencies*>(sample->getOrigFrequencies().data());
		for (uint i=0; i<freqCount; ++i){
			maxValue = std::max(maxValue, origFrequencies[i].maxValue);
			minValue = std::min(minValue, origFrequencies[i].minValue);
		}
		// printf("%g; %g\n", minValue, maxValue);
		frequencies = NULL;
	} else {
		maxFreq = COUNT_FREQ-1;
		frequencies = const_cast<SFrequencies*>(sample->getFrequencies().data());
		origFrequencies = NULL;
	}
	update();
}

void CSpectrogram::init(){
	freqCount = 0;
	frequencies = NULL;
	origFrequencies = NULL;
	maxFreq = 0;
	// setAutoFillBackground(true);
	// QPalette pal = palette();
	// pal.setColor(QPalette::Window, Qt::black);
	// setPalette(pal);
	xStart = yStart = 0;
	// colorer = new CBlackWhiteColor();
	// colorer = new CWhiteBlackColor();
	// colorer = new CTryColor();
}

void CSpectrogram::setDescription(const QString& txt){
	description = txt;
	update();
}
