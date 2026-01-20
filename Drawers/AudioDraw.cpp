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

#include "AudioDraw.hxx"
#include <QPainter>
#include <cmath>

using namespace std;

void AudioDraw::mousePressEvent(QMouseEvent *event){
	if (selectable && event->button() == Qt::LeftButton && !selecting && size > 0) {
		selection.start = event->x();
		selection.end = -1;
		selecting = true;
		update();
	}
}

void AudioDraw::mouseMoveEvent(QMouseEvent *event){
	if (selecting){
		int low = min(event->x(), min(selection.start, selection.end)) - 1;
		int high = max(event->x(), max(selection.start, selection.end)) + 1;
		selection.end = event->x();
		update(low, 0, high-low, height());
		//		update();
	}
}

void AudioDraw::mouseReleaseEvent(QMouseEvent *event){
	if (event->button() == Qt::LeftButton && selecting) {
		selection.end = event->x();
		if (selection.start > selection.end){
			swap(selection.start, selection.end);
		}
		selecting = false;
	} else if (event->button() == Qt::RightButton){
		selecting = false;
		if (selection.start != selection.end){
			zoomSelected();
		} else if (selectable) {
			zoomOut();
		}
	} else if (event->button() == Qt::LeftButton){
		// printf("Emitting sample selected on pos: %d (X: = %d)\n", panelToFrame(event->x()), event->x());
		sRegion reg = findSample(panelToFrame(event->x()));
		emit samplesSelected(reg.start, reg.end);
	} 
	if (event->button() == Qt::MidButton && selection.start != selection.end){
		emitSelected();
	}
}

void AudioDraw::paintEvent(QPaintEvent* event) {
	QRect r = event->rect();
	//	printf("%d %d %d %d\n", r.x(), r.y(), r.width(), r.height());
	if (size <= 0){
		// paint.drawLine(0, 0, r.width(), r.height());
		return;
	} 

	vector<double> & values = *pValues;
	QPainter paint(this);
	paint.setPen(Qt::black);
	int myHeight = height() - LEGEND_HEIGHT - 1;
	int step = panelToFrame(1) - panelToFrame(0);
	if (step > 8) {
		int count = max(step/32, 1);
		for (int i=r.x(); i<r.width()+r.x(); ++i) {
			int x = panelToFrame(i);
			if (x >= (int)values.size()){
				break;
			}
			if (x >= chosen.end){
				paint.setPen(Qt::black);
			} else if (x >= chosen.start){
				paint.setPen(Qt::red);
			}
			double maxV = 0;
			for (int j=0; j<step; j+=count){
				maxV = max(maxV,abs(values[x+j]));
			}
			int from = (int)((-(maxV*zoomY)+1.0)*myHeight/2);
			int to = (int)(((maxV*zoomY)+1.0)*myHeight/2);
			to = min (to, myHeight);
			paint.drawLine(i, from, i, to);
		}
	} else {
		int x = panelToFrame(r.x());
		int lastY = (int)((-(values[x]*zoomY)+1.0)*myHeight/2);
		int xTo = panelToFrame(r.x()+r.width()+2);
		if (xTo > (int)values.size()){
			xTo = values.size();
		}
		for (++x; x<=xTo; ++x) {
			if (x >= chosen.end){
				paint.setPen(Qt::black);
			} else if (x >= chosen.start){
				paint.setPen(Qt::red);
			}
			int y = (int)((-(values[x]*zoomY)+1.0)*myHeight/2);
			paint.drawLine(frameToPanel(x-1), lastY, frameToPanel(x), min(y, myHeight));
			lastY = y;
		}
	}
	if (selectedRegions.size() > 0){	
		QPen pen(Qt::darkGreen);
		pen.setWidth(1);
		QBrush brush(QColor(0, 255, 0,  35));
		paint.setPen(pen);
		paint.setBrush(brush);
		for (list<sRegion>::iterator it = selectedRegions.begin(); it != selectedRegions.end(); ++it){
			int start = frameToPanel(it->start);
			int end = frameToPanel(it->end);
			if (end < 0 || start > width()) {
				continue;
			}
			if (start < 0){
				start = 0;
			}
			if (end > width()){
				end = width();
			}
			//printf("[%d %d] Width: %d; Start: %d -> %d; End: %d -> %d\n", viewRegion.start, viewRegion.end, width(), it->start, start, it->end, end);
			if (end >= r.x() || start <= r.x()+width()){
				paint.drawRect(start, 0, end-start, myHeight-1);
			}
		}
	}
	if (selection.start != -1 && selection.end == -1){
		paint.drawLine(selection.start, 0, selection.start, myHeight-1);
	}
	if (selection.end != -1){
		QBrush brush(QColor(0, 0, 255,  25));
		paint.setBrush(brush);
		QPen pen(Qt::darkBlue);
		pen.setWidth(1);
		paint.setPen(pen);
		paint.drawRect(selection.start, 0, selection.end-selection.start, myHeight-1);
	}
	if (marker != -1){
		paint.setPen(Qt::red);
		int mx = frameToPanel(marker);
		if (mx >= r.x() && mx < r.x() + r.width()){
			paint.drawLine(mx, 0, mx, myHeight);
		}
		markerDrawedPos = marker;
	}
	paint.setPen(Qt::black);
	paint.drawLine(0, myHeight, width(), myHeight);
	const int STEP = 40;
	int start = ((long long)viewRegion.start) * width() / (viewRegion.end - viewRegion.start);
	start %= STEP;
	// printf("Start: %d, vrS: %d, ftp: %d\n", start, viewRegion.start, frameToPanel(viewRegion.start));
	for (int i=-start; i<width(); i+=STEP){
		int frame = panelToFrame(i);
		QString timeLabel = QString::number(1.0 * frame / 44100, 'f', 2);
		paint.drawLine(i, myHeight+1, i, height()-12);
		// paint.drawText(i, height()-1, timeLabel);
		paint.drawText(i-STEP/2+1, height()-10, STEP-1, 10, Qt::AlignCenter, timeLabel);
	}
}

/*
	 void AudioDraw::resizeEvent(QResizeEvent *event){
	 }
 */

void AudioDraw::setSignal(vector<double> &values){
	this->size = values.size();
	this->pValues = &values;
	changeViewRegion(0, size);
	selectedRegions.clear();
	chosen.start = chosen.end = -1;
	update();
}

void AudioDraw::setZoomY(int zoom){
	zoomY = (zoom-200)/10;
	if (zoomY < 1 && zoomY >= 0){
		zoomY = 1;
	}
	if (zoomY < 0){
		zoomY = 1/(-zoomY);
	}
	update();
}

void AudioDraw::setSelectable(bool s){
	selectable = s;
}

void AudioDraw::changeViewRegion(int s, int e){
	// printf("%s: %d->%d, %d->%d\n", this->objectName().toStdString().c_str(), viewRegion.start, s, viewRegion.end, e);
	if (viewRegion.start == s && viewRegion.end == e){
		return;
	}
	if (s > e){
		return;
	}
	// if (e > size){
	// 	e = size - 1;
	// }
	viewRegion.start = s;
	viewRegion.end = e;
	selection.start = selection.end = -1;
	markerDrawedPos = s;
	emit viewRegionChanged(viewRegion.start, viewRegion.end);
	update();
}

void AudioDraw::changeViewRegionStart(int s){
	// printf("%s: %d->%d, %d->\n", this->objectName().toStdString().c_str(), viewRegion.start, s, viewRegion.end);
	if (s == viewRegion.start){
		return;
	}
	if (s < 0){
		return;
	}
	int delta = viewRegion.start - s;
	viewRegion.end -= delta;
	viewRegion.start = s;
	// selection.start -= frameToPanel(delta);
	// selection.end -= frameToPanel(delta);
	selection.start = selection.end = -1;
	markerDrawedPos = s;
	emit viewRegionChanged(viewRegion.start, viewRegion.end);
	update();
}

void AudioDraw::addSelection(int start, int end){
	selectedRegions.push_back(sRegion(start, end));
}

sRegion AudioDraw::getSelection(){
	if (selection.end == -1){
		return sRegion(-1, -1);
	}
	return sRegion(panelToFrame(selection.start), panelToFrame(selection.end));
}

void AudioDraw::setChosen(int s, int e){
	chosen.start = s;
	chosen.end = e;
	update();
}

void AudioDraw::init(){
	size = 0;
	zoomY = 1;
	selection.start = -1;
	selection.end = -1;
	selecting = false;
	selectable = true;
	marker = -1;
	markerDrawedPos = 0;
}

void AudioDraw::setSelection(list<sRegion>& sel){
	selectedRegions = sel;
	update();
}

sRegion AudioDraw::findSample(int frame){
	// printf("Frame: %d\n", frame);
	for (list<sRegion>::iterator it = selectedRegions.begin(); it != selectedRegions.end(); ++it){
		if (it->start <= frame && it->end >= frame){
			return *it;
		}
	}
	return sRegion();
}

void AudioDraw::setMarker(int pos){
	int opos = frameToPanel(markerDrawedPos);
	int npos = frameToPanel(pos);
	marker = pos;
	if (opos < 0){
		opos = 0;
		markerDrawedPos = 0;
	}
	if (opos == npos)
		return;
	if (opos > npos){
		update();
	} else if (npos != opos ){
		update(opos, 0, npos, height());
	}
}

void AudioDraw::zoom11(){
	viewRegion.start = 0;
	viewRegion.end = size;
	emit viewRegionChanged(viewRegion.start, viewRegion.end);
	update();
}

void AudioDraw::zoomSelected(){
	if (selection.start != selection.end){
		int low = selection.start;
		low = max(0, low);
		int high = selection.end;
		high = min(width(), high);
		int rWidth = viewRegion.end - viewRegion.start;
		int oldStart = viewRegion.start;
		viewRegion.start = oldStart + (long long)low*rWidth/width();
		viewRegion.end = oldStart + (long long)high*rWidth/width();
		if (viewRegion.end == viewRegion.start){
			--viewRegion.end;
		}
		emit viewRegionChanged(viewRegion.start, viewRegion.end);
		selection.start = selection.end = -1;
		//printf("%d %d %d %d %d %d\n", low, high, rWidth, width(), viewRegion.start, viewRegion.end);
		update();
	} else {
		int delta = viewRegion.end - viewRegion.start;
		delta /= 3;
		viewRegion.start += delta;
		viewRegion.end -= delta;
		emit viewRegionChanged(viewRegion.start, viewRegion.end);
		update();
	}
};

void AudioDraw::emitSelected(){
	if (selection.start != selection.end){
		emit samplesSelected(panelToFrame(selection.start), panelToFrame(selection.end));
	}
}

void AudioDraw::zoomOut(){
	int delta = viewRegion.end - viewRegion.start;
	delta /= 2;
	viewRegion.start -= delta;
	viewRegion.end += delta;
	if (viewRegion.start < 0){
		viewRegion.end += -viewRegion.start;
		viewRegion.start = 0;
	}
	if (viewRegion.end >= size){
		int tmp = viewRegion.end - size + 1;
		viewRegion.end -= tmp;
		viewRegion.start -= tmp;
	}
	if (viewRegion.start < 0){
		viewRegion.start = 0;
	}
	emit viewRegionChanged(viewRegion.start, viewRegion.end);
	update();
}
