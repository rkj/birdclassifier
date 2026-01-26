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

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

using namespace std;

namespace {
bool bscDebugEnabled() {
	static int enabled = -1;
	if (enabled < 0) {
		const char* env = std::getenv("BSC_DEBUG");
		enabled = (env && *env) ? 1 : 0;
	}
	return enabled == 1;
}

void bscDebugLog(const char* fmt, ...) {
	if (!bscDebugEnabled()) {
		return;
	}
	va_list args;
	va_start(args, fmt);
	std::fprintf(stderr, "BSC: ");
	std::vfprintf(stderr, fmt, args);
	std::fprintf(stderr, "\n");
	va_end(args);
}

void bscProcessEvents(size_t i, size_t step) {
	if (step == 0) {
		return;
	}
	if (i % step == 0) {
		QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
	}
}
} // namespace

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
	QElapsedTimer timer;
	timer.start();
	bscDebugLog("Energy draw: computing %zu buckets.", powers.size());
	const size_t updateStep = std::max<size_t>(1, powers.size() / 100);
	const size_t logStep = std::max<size_t>(1, powers.size() / 10);
	for (size_t i=0; i<powers.size(); i++){
		powers[i] = powerTodB(computePower(values, i*DELTA, DELTA));
		bscProcessEvents(i, updateStep);
		if (bscDebugEnabled() && i % logStep == 0) {
			bscDebugLog("Energy draw: power %zu/%zu.", i, powers.size());
		}
	}
	bscDebugLog("Energy draw: computed powers in %lld ms.", timer.elapsed());
	viewRegion.start = 0;
	viewRegion.end = values.size();
	timer.restart();
	updateSelection();
	bscDebugLog("Energy draw: selection computed in %lld ms.", timer.elapsed());
	timer.restart();
	refreshSeries();
	bscDebugLog("Energy draw: series refreshed in %lld ms.", timer.elapsed());
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
	refreshSeries();
	update();
}

void CEnergyDraw::paintEvent(QPaintEvent *event){
	QChartView::paintEvent(event);
}

void CEnergyDraw::changeViewRegion(int s, int e){
	// printf("Energy: %d %d (max: %d)\n", s, e, powers.size() * DELTA);
	if ((viewRegion.start == s && viewRegion.end == e) || (e/DELTA > (int)powers.size())){
		return;
	}
	viewRegion.start = s;
	viewRegion.end = e;
	if (axisX != nullptr && viewRegion.end > viewRegion.start){
		axisX->setRange(viewRegion.start / 44100.0, viewRegion.end / 44100.0);
	}
	refreshCutoff();
	update();
}

void CEnergyDraw::init(){
	viewRegion.start = -1;
	viewRegion.end = -1;
	changes = true;
	cutOff = 0;
	chart = new QChart();
	series = new QLineSeries(chart);
	cutoffSeries = new QLineSeries(chart);
	cutoffSeries->setColor(Qt::red);
	axisX = new QValueAxis(chart);
	axisY = new QValueAxis(chart);
	axisX->setTitleText("Time (s)");
	axisY->setTitleText("Power (dB)");
	axisY->setRange(0.0, 125.0);
	chart->addSeries(series);
	chart->addSeries(cutoffSeries);
	chart->addAxis(axisX, Qt::AlignBottom);
	chart->addAxis(axisY, Qt::AlignLeft);
	series->attachAxis(axisX);
	series->attachAxis(axisY);
	cutoffSeries->attachAxis(axisX);
	cutoffSeries->attachAxis(axisY);
	chart->legend()->hide();
	setChart(chart);
	setRenderHint(QPainter::Antialiasing);
}

CEnergyDraw::~CEnergyDraw(){
}

void CEnergyDraw::changeCutOff(int val){
	cutOff = val - LEGEND_HEIGHT;
	changes = true;
	updateSelection();
	refreshCutoff();
	update();
}

void CEnergyDraw::updateSelection(){
	selection.clear();
	int start = -1;
	const size_t updateStep = std::max<size_t>(1, powers.size() / 100);
	const size_t logStep = std::max<size_t>(1, powers.size() / 10);
	for (size_t i=0; i<powers.size(); ++i){
		if (powers[i] > cutOff && start == -1){
			start = i*DELTA;
		} else if (powers[i] < cutOff && start != -1){
			selection.push_back(sRegion(start, i*DELTA-1));
			start = -1;
		}
		bscProcessEvents(i, updateStep);
		if (bscDebugEnabled() && i % logStep == 0) {
			bscDebugLog("Energy draw: selection %zu/%zu.", i, powers.size());
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

void CEnergyDraw::refreshSeries(){
	if (series == nullptr || axisX == nullptr){
		return;
	}
	series->clear();
	const size_t updateStep = std::max<size_t>(1, powers.size() / 100);
	const size_t logStep = std::max<size_t>(1, powers.size() / 10);
	for (size_t i = 0; i < powers.size(); ++i){
		double x = (i * DELTA) / 44100.0;
		series->append(x, powers[i]);
		bscProcessEvents(i, updateStep);
		if (bscDebugEnabled() && i % logStep == 0) {
			bscDebugLog("Energy draw: series %zu/%zu.", i, powers.size());
		}
	}
	if (viewRegion.end > viewRegion.start){
		axisX->setRange(viewRegion.start / 44100.0, viewRegion.end / 44100.0);
	}
	refreshCutoff();
}

void CEnergyDraw::refreshCutoff(){
	if (cutoffSeries == nullptr || viewRegion.end <= viewRegion.start){
		return;
	}
	cutoffSeries->clear();
	const double startX = viewRegion.start / 44100.0;
	const double endX = viewRegion.end / 44100.0;
	cutoffSeries->append(startX, cutOff);
	cutoffSeries->append(endX, cutOff);
}
