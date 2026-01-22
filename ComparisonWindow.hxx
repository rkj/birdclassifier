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

#ifndef _COMPARISONWINDOW_HXX
#define _COMPARISONWINDOW_HXX

#include "ui_ComparisonWindow.h"
#include <vector>
#include "Audio.hxx"

// Note: Do not use "using namespace std" in headers
// Use std:: prefix explicitly to avoid namespace pollution

class ComparisonWindow : public QDialog, public Ui::ComparisonWindow {
	Q_OBJECT

	public:
		static uint COUNT;
		ComparisonWindow(QWidget *parent, CSample* sample, std::vector<CSample*>& learn);
		~ComparisonWindow();
	protected:

	private:
		std::vector<CSample*> learning;
		std::vector<SpectroDraw*> spectros;

	private slots:

	private:
};

#endif // _COMPARISONWINDOW_HXX
