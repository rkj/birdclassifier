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

#ifndef _ENERGYDRAW_HXX
#define _ENERGYDRAW_HXX

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include "AudioDraw.hxx"
#include <list>
#include <vector>

const int DELTA = 64;
class CEnergyDraw : public QtCharts::QChartView {
	Q_OBJECT
	public:
		static const int LEGEND_WIDTH = 50;
		static const int LEGEND_HEIGHT = 20;
		void setSignal(std::vector<double> &values);
		void updateSignal(std::vector<double>& values);
		CEnergyDraw(){
			init();
		}
		CEnergyDraw(QWidget* w) : QtCharts::QChartView(w) {
			init();
		}
		CEnergyDraw(QGroupBox* &gb) : QtCharts::QChartView(gb){
			init();
		}
		~CEnergyDraw();
		int frameToPanel(int x){
			if (viewRegion.end <= viewRegion.start || chart == nullptr){
				return 0;
			}
			const QRectF area = chart->plotArea();
			if (area.width() <= 0.0){
				return 0;
			}
			return static_cast<int>(area.left() + ((long long)x - viewRegion.start) * area.width() / (viewRegion.end - viewRegion.start));
		};
		int panelToFrame(int x){
			if (viewRegion.end <= viewRegion.start || chart == nullptr){
				return 0;
			}
			const QRectF area = chart->plotArea();
			if (area.width() <= 0.0){
				return viewRegion.start;
			}
			return static_cast<int>((long long)(x - area.left()) * (viewRegion.end - viewRegion.start) / area.width() + viewRegion.start);
		};
	protected:
		void paintEvent(QPaintEvent *event);
		void mouseReleaseEvent(QMouseEvent *event);
		void mouseMoveEvent(QMouseEvent *event);

	public slots:
		void changeViewRegion(int, int);
		void changeCutOff(int);
	signals:
		void selectedFragments(std::list<sRegion>&);
		void selectedVisibleFragments(std::list<sRegion>&);

	private:
		void init();
		void refreshSeries();
		void refreshCutoff();
		std::vector<double> powers;
		QtCharts::QChart* chart;
		QtCharts::QLineSeries* series;
		QtCharts::QLineSeries* cutoffSeries;
		QtCharts::QValueAxis* axisX;
		QtCharts::QValueAxis* axisY;
		sRegion viewRegion;
		int cutOff;
		std::list<sRegion> selection;
		bool changes;
		void updateSelection();
		void updateSelection(int);
};


#endif //ENERGYDRAW_HXX
