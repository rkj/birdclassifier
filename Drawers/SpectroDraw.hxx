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

#ifndef _SPECTRODRAW_HXX
#define _SPECTRODRAW_HXX

#include "ui_SpectroDraw.h"
#include <QtDesigner/QDesignerExportWidget>

class SpectroDraw : public QWidget, private Ui::SpectroDraw {
	Q_OBJECT
	public:
		SpectroDraw(QWidget *parent = 0);
		SpectroDraw(QGroupBox *parent = 0);
		void setSample(CSample *, bool original = true);
		void setDescription(const QString& txt){
			spectrogram->setDescription(txt);
		}

	protected:
		void resizeEvent (QResizeEvent * event);
	private:
		void init();
		void tuneSliders();
		int freqCount;
		bool original;
	
	public slots:
	signals:
};

#endif// _SPECTRODRAW_HXX
