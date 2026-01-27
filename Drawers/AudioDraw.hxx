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

#ifndef _HXX_AUDIODRAW
#define _HXX_AUDIODRAW

#include <QLabel>
#include <QGroupBox>
#include <QPaintEvent>
#include <list>
#include <vector>

// Note: Do not use "using namespace std" in headers
// Use std:: prefix explicitly to avoid namespace pollution
struct sRegion{
	int start;
	int end;
	sRegion(){
		start = end = -1;
	}
	sRegion(int s, int e) : start(s), end(e){
	}
};

class AudioDraw : public QLabel {
	Q_OBJECT
	public:
		static const int LEGEND_HEIGHT = 20;
		void setSignal(std::vector<double> &values);
		void setSignal(){
			size = 0;
			update();
		}
		void setSelectable(bool s);
		void setSelected(std::list<sRegion> regions);
		void addSelection(int start, int end);
		void setMarker(int pos);
		sRegion getViewRegion(){
			return viewRegion;
		}

		void setViewRegion(sRegion region);
		sRegion getSelection();

		AudioDraw(){
			init();
		}
		AudioDraw(QWidget* w) : QLabel(w) {
			init();
		}
		AudioDraw(QGroupBox* &gb) : QLabel(gb){
			init();
		}
		~AudioDraw(){
		}
		int frameToPanel(int x){
			// return (int) ((1.0*x - viewRegion.start) * width() / (viewRegion.end - viewRegion.start));
			return ((long long)x - viewRegion.start) * width() / (viewRegion.end - viewRegion.start);
		};
		int panelToFrame(int x){
			return (long long)x*(viewRegion.end-viewRegion.start)/width() + viewRegion.start;
			// return (int)(1.0*x*(viewRegion.end-viewRegion.start)/width() + viewRegion.start);
		};
	protected:
		void mousePressEvent(QMouseEvent *event);
		void mouseMoveEvent(QMouseEvent *event);
		void mouseReleaseEvent(QMouseEvent *event);
		void paintEvent(QPaintEvent *event);

	private:
		int markerDrawedPos;
		std::vector<double> *pValues;
		int size;
		double zoomY;
		std::list<sRegion> selectedRegions;
		sRegion viewRegion;
		sRegion selection;
		sRegion chosen;
		bool selecting;
		bool selectable;
		int marker;
		void init();
		sRegion findSample(int frame);

	public slots:
		void changeViewRegion(int, int);
		void changeViewRegionStart(int);
		void setZoomY(int zoom); //zoom 0.1
		void setChosen(int, int);
		void setSelection(std::list<sRegion>&);
		void zoomSelected();
		void zoom11();
		void zoomOut();
		void emitSelected();
signals:
		void viewRegionChanged(int, int);
		void samplesSelected(int, int);
};

#endif
