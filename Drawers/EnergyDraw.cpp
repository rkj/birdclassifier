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

#include "EnergyDraw.hxx"
#include "../detect/Audio.hxx"
#include <QPainter>

using namespace std;

double computePower(vector<double> &val, int start, int count){
	double energy = 0;
	for (int i=0; i<count; i++){
		energy += val[i+start]*val[i+start];
	}
	return energy/count;
}

void CEnergyDraw::setSignal(vector<double> &values){
	powers.clear();
	powers.resize(values.size()/DELTA);
	for (uint i=0; i<values.size()/DELTA; i++){
		powers[i] = powerTodB(computePower(values, i*DELTA, DELTA));
		// printf("[%d]: %g\n", i, powers[i]);
	}
	viewRegion.start = 0;
	viewRegion.end = values.size();
	updateSelection();
	update();
}

void CEnergyDraw::updateSignal(vector<double>& values){
	if (powers.size() * DELTA > values.size()){
		setSignal(values);
		return;
	}
	// int oldsize = powers.size()*DELTA;
	powers.resize(values.size());
	for (uint i=powers.size(); i<values.size()/DELTA; i++){
		powers[i] = powerTodB(computePower(values, i*DELTA, DELTA));
	}
	viewRegion.start = 0;
	viewRegion.end = values.size();
	// updateSelection(oldsize);
	update();
}

void CEnergyDraw::paintEvent(QPaintEvent *event){
	if (powers.size() <= 0){
		return;
	} 
	QRect r = event->rect();
	QPainter paint(this);
	paint.setPen(Qt::black);
	// int lastY = height()-1;
	// for (int i=0; i<width(); ++i){
	// 	int y = height()-(int)powers[i*10];
	// 	paint.drawLine(i-1, lastY, i, y);
	// 	lastY = y;
	// }

	int myHeight = height() - LEGEND_HEIGHT;
	int step = panelToFrame(1) - panelToFrame(0);
	step /= DELTA;
	const int MAGIC = 4;
	if (step > 1) {
		int count = max(step/MAGIC, 1);
		int lastY = powerToY(powers[panelToFrame(max(r.x()-1, 0))/DELTA]);
		if (lastY < powerToY(cutOff)){
			paint.setPen(Qt::red);
		}
		for (int i=r.x(); i<r.width()+r.x(); ++i) {
			int x = panelToFrame(i) / DELTA;
			double maxV = 0;
			for (int j=0; j<step; j+=count){
				maxV = max(maxV,abs(powers[x+j]));
			}
			int y = powerToY(maxV);
			int pY = powerToY(cutOff);
			if (y > pY && lastY <= pY) { //zmiana w dol
				int crossX;
				if (y - pY > pY - lastY){
					crossX = i-1;
				} else {
					crossX = i;
				}
				paint.drawLine(i-1, lastY, crossX, pY);
				paint.setPen(Qt::black);
				paint.drawLine(crossX, pY, i, y);
			} else if (y <= pY && lastY > pY){ //zmiana w gore
				int crossX;
				if (pY - y > lastY - pY){
					crossX = i-1;
				} else {
					crossX = i;
				}
				paint.drawLine(i-1, lastY, crossX, pY);
				paint.setPen(Qt::red);
				paint.drawLine(crossX, pY, i, y);
			}	else if (y > pY && lastY > pY){	//oba na dole
				// paint.setPen(Qt::black);
				paint.drawLine(i-1, lastY, i, y);
			} else { //oba na gorze
				// paint.setPen(Qt::red);
				paint.drawLine(i-1, lastY, i, y);
			}
			// printf("MaxV: %g\n", maxV);
			// paint.drawLine(i-1, lastY, i, y);
			lastY = y;
		}
	} else {
		int x = max(panelToFrame(r.x()-1), 0);
		int lastY = powerToY(powers[x/DELTA]);
		if (lastY < powerToY(cutOff)){
			paint.setPen(Qt::red);
		}
		for (++x; x<=panelToFrame(r.x()+r.width()+2); ++x) {
			int y = powerToY(powers[x/DELTA]);
			if (y > powerToY(cutOff) && lastY > powerToY(cutOff)){
				paint.setPen(Qt::black);
			} else {
				paint.setPen(Qt::red);
			}
			paint.drawLine(frameToPanel(x-1), lastY, frameToPanel(x), y);
			lastY = y;
		}
	}
	paint.setPen(Qt::red);
	paint.drawLine(0, powerToY(cutOff), width(), powerToY(cutOff));

	//Horizontal legend
	paint.setPen(Qt::black);
	paint.drawLine(0, myHeight, width(), myHeight);
	const int STEP = 40;
	int start = ((long long)viewRegion.start) * width() / (viewRegion.end - viewRegion.start);
	start %= STEP;
	// printf("Start: %d, vrS: %d, ftp: %d\n", start, viewRegion.start, frameToPanel(viewRegion.start));
	for (int i=-start; i<width(); i+=STEP){
		int frame = panelToFrame(i);
		char buf[10];
		sprintf(buf, "%.2f", 1.0*frame/44100);
		paint.drawLine(i, myHeight, i, height()-12);
		// paint.drawText(i, height()-1, buf);
		paint.drawText(i-STEP/2+1, height()-10, STEP-1, 10, Qt::AlignCenter, buf);
	}

	//Vertical legend
	QColor color = paint.background().color();
	color.setAlpha(200);
	paint.setBrush(QBrush(color));
	paint.setPen(Qt::NoPen);
	paint.drawRect(0, 0, LEGEND_WIDTH, height());

	paint.setPen(Qt::black);
	paint.setBrush(Qt::NoBrush);
	paint.drawLine(LEGEND_WIDTH-3, 0, LEGEND_WIDTH-3, myHeight);
	for (int y=myHeight; y>=0; y-=20){
		paint.drawLine(LEGEND_WIDTH-3, y, LEGEND_WIDTH-12, y);
		char buf[4];
		sprintf(buf, "%.1f", yToPower(y));
		paint.drawText(0, y-7, LEGEND_WIDTH-13, 20, Qt::AlignRight, buf);
	}
	color.setAlpha(150);
	paint.setBrush(QBrush(color));
	paint.setPen(Qt::NoPen);
	paint.drawRect(0, 0, 25, 14);
	paint.setPen(Qt::red);
	paint.drawText(0, 0, 25, 15, Qt::AlignLeft | Qt::AlignTop, "[dB]");
}

void CEnergyDraw::changeViewRegion(int s, int e){
	// printf("Energy: %d %d (max: %d)\n", s, e, powers.size() * DELTA);
	if ((viewRegion.start == s && viewRegion.end == e) || (e/DELTA > (int)powers.size())){
		return;
	}
	viewRegion.start = s;
	viewRegion.end = e;
	update();
}

void CEnergyDraw::init(){
	viewRegion.start = -1;
	viewRegion.end = -1;
	changes = true;
}

CEnergyDraw::~CEnergyDraw(){
}

inline double CEnergyDraw::yToPower(int y){
	int mh = height() - LEGEND_HEIGHT;
	return -125.0*(y - mh) / mh;
}

inline int CEnergyDraw::powerToY(double val){
	int mh = height() - LEGEND_HEIGHT;
	return mh - (int)(mh*val/125);
}

void CEnergyDraw::changeCutOff(int val){
	cutOff = val - LEGEND_HEIGHT;
	changes = true;
	updateSelection();
	update();
}

void CEnergyDraw::updateSelection(){
	selection.clear();
	int start = -1;
	for (uint i=0; i<powers.size(); ++i){
		if (powers[i] > cutOff && start == -1){
			start = i*DELTA;
		} else if (powers[i] < cutOff && start != -1){
			selection.push_back(sRegion(start, i*DELTA-1));
			start = -1;
		}
	}
	if (start != -1) {
		selection.push_back(sRegion(start, (powers.size())*DELTA-1));
	}
	emit selectedFragments(selection);
} 

void CEnergyDraw::updateSelection(int from){
	int start = -1;
	sRegion last = selection.back();
	selection.pop_back();
	if (last.end >= (int)(powers.size()-2)*DELTA){
		start = last.start;
	}
	for (uint i=from; i<powers.size(); ++i){
		if (powers[i] > cutOff && start == -1){
			start = i*DELTA;
		} else if (powers[i] < cutOff && start != -1){
			selection.push_back(sRegion(start, i*DELTA-1));
			start = -1;
		}
	}
	if (start != -1) {
		selection.push_back(sRegion(start, (powers.size())*DELTA-1));
	}
	emit selectedFragments(selection);
}

void CEnergyDraw::mouseReleaseEvent(QMouseEvent *){
}

void CEnergyDraw::mouseMoveEvent(QMouseEvent *){
	//	emit energyTxt(
}

