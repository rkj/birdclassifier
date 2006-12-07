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

#include "ComparisonWindow.hxx"
#include <algorithm>

struct CCompareResults{
	double diff;
	CSample *sample;
};

bool operator<(const CCompareResults &a, const CCompareResults &b){
		return a.diff < b.diff;
}

uint ComparisonWindow::COUNT = 5;
ComparisonWindow::ComparisonWindow(QWidget *parent, CSample* sample, vector<CSample*>& learn) : QDialog(parent) {
	setupUi(this);
	spectroDraw->setSample(sample, false);
	QVBoxLayout * myVBox = new QVBoxLayout(frame);
	myVBox->setSpacing(1);
	myVBox->setMargin(1);
	myVBox->setObjectName(QString::fromUtf8("myVBox"));

	CCompareResults cr[learn.size()];
	for (uint i=0; i<learn.size(); i++){
		cr[i].sample = learn[i];
		cr[i].diff = cr[i].sample->differ(*sample);
	}
	sort (&cr[0], &cr[learn.size()]);
	spectros.resize(COUNT);
	QString txt;
	char buf[50];
	for (uint i=0; i<COUNT; i++){
		spectros[i] = new SpectroDraw(frame);
		spectros[i]->setObjectName(QString("learningSpectro") + i);
    spectros[i]->setMinimumSize(QSize(250, 160));
		spectros[i]->setSample(cr[i].sample, false);
		sprintf(buf, "%s (%3.2g)", cr[i].sample->getName().c_str(), cr[i].diff);
		spectros[i]->setDescription(buf);
		myVBox->addWidget(spectros[i]);
		sprintf(buf, "Spectro%d (%f)\n", i, cr[i].diff);
		txt += buf;
		int count = min(cr[i].sample->getFreqCount(), sample->getFreqCount());
		for (int j=0; j<count; j++){
			double tmp = sample->getFrequencies()[j].differ(cr[i].sample->getFrequencies()[j]);
			sprintf(buf, "[%d] = %f\n", j, tmp);
			txt += buf;
		}
		txt += '\n';
	}
	textBrowser->setPlainText(txt);
}

ComparisonWindow::~ComparisonWindow(){
	for (vector<SpectroDraw*>::iterator it = spectros.begin(); it != spectros.end(); it++){
		delete *it;
	}
}
