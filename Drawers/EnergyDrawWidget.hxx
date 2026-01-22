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

#ifndef _ENERGYDRAWWIDGET_HXX
#define _ENERGYDRAWWIDGET_HXX

#include "ui_EnergyDraw.h"
#include <QGroupBox>

class EnergyDrawWidget : public QWidget, private Ui::EnergyDrawWidget {
	Q_OBJECT
	public:
		EnergyDrawWidget(QWidget * parent = 0) : QWidget(parent){
			init();
		};
		EnergyDrawWidget(QGroupBox * parent = 0) : QWidget(parent){
			init();
		}
		CEnergyDraw* getEnergy(){
			return energy;
		}

	protected:
	private:
		void init();

	public slots:
	signals:
};

#endif// _ENERGYDRAWWIDGET_HXX
