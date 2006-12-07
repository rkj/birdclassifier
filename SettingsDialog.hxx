/*
		QTDetection, bird voice visualization and comaprison.
		Copyright (C) 2006 Roman Kamyk, All rights reserved.
 
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.
 
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
 
    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "ui_SettingsDialog.h"
#include <QAbstractListModel>
#include <vector>
#include <QAbstractListModel>
#include "Spectrogram.hxx"

using namespace std;


class ColorListModel : public QAbstractListModel
{
	Q_OBJECT

	public:
		ColorListModel(vector<CSpectColor*>& _colorList, QObject *parent = 0)	: QAbstractListModel(parent), colorList(_colorList){
		}

		int rowCount(const QModelIndex & = QModelIndex()) const {
			return colorList.size();
		}

		QVariant data(const QModelIndex &index, int role) const {
			if (!index.isValid())
				return QVariant();

			if ((int)index.row() >= (int)colorList.size())
				return QVariant();

			if (role == Qt::DisplayRole || role == Qt::EditRole) {
				return colorList[index.row()]->description();
			} else if (role == 9999){
				return (int)(colorList[index.row()]);
			} else {
				return QVariant();
			}
		}

		Qt::ItemFlags flags ( const QModelIndex & index ) const {
			return QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
		}

		QVariant headerData(int section, Qt::Orientation orientation,	int role = Qt::DisplayRole) const {
			if (role != Qt::DisplayRole)
				return QVariant();

			if (orientation == Qt::Horizontal)
				return QString("Column %1").arg(section);
			else
				return QString("Row %1").arg(section);
		}

	private:
		vector<CSpectColor*> colorList;
};

class SettingsDialog : public QDialog, public Ui::SettingsDialog {
	Q_OBJECT

	public:
		SettingsDialog(QWidget *parent) : QDialog(parent){
			init();
		}
		~SettingsDialog();
	protected:

	private:

		private slots:

	private:
			void init();
};
