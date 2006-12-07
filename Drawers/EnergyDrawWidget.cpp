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

#include "EnergyDrawWidget.hxx"
#include "../detect/Audio.hxx"

extern double POWER_CUTOFF;
void EnergyDrawWidget::init(){
	setupUi(this);

	connect( cutOff, SIGNAL(valueChanged(int)), energy, SLOT(changeCutOff(int)) );

	// cutOff->setValue(powerTodB(POWER_CUTOFF));
	cutOff->setValue(74+CEnergyDraw::LEGEND_HEIGHT);
}
