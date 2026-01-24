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

#ifndef _LEARNINGDIALOG_HXX
#define _LEARNINGDIALOG_HXX

#include "ui_LearningDialog.h"
#include <QAbstractListModel>
#include <vector>
#include <memory>
#include "Spectrogram.hxx"

// Note: Do not use "using namespace std" in headers
// Use std:: prefix explicitly to avoid namespace pollution


class LearningListModel : public QAbstractListModel
{
	Q_OBJECT

	public:
		LearningListModel(std::vector<std::unique_ptr<CSample>>& _learning, QObject *parent = 0)	: QAbstractListModel(parent), learning(&_learning){
		}

		int rowCount(const QModelIndex & = QModelIndex()) const {
			return learning->size();
		}

		QVariant data(const QModelIndex &index, int role) const {
			if (!index.isValid())
				return QVariant();

			if ((int)index.row() >= (int)learning->size())
				return QVariant();

			if (role == Qt::DisplayRole || role == Qt::EditRole) {
				return ((*learning)[index.row()]->getBirdAndId()).c_str();
			} else if (role == 9999){
				return QVariant::fromValue(reinterpret_cast<quintptr>((*learning)[index.row()].get()));
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

		bool removeRows (int row, int count, const QModelIndex& parent = QModelIndex()) {
			beginRemoveRows(parent, row, row+count);
			learning->erase(learning->begin()+row, learning->begin()+row+count);
			endRemoveRows();
			return true;
		}
	private:
		std::vector<std::unique_ptr<CSample>>* learning;
};

class LearningDialog : public QDialog, public Ui::LearningDialog {
	Q_OBJECT

	public:
		LearningDialog(QWidget *parent) : QDialog(parent){
			init();
		}
		~LearningDialog();
		void connectAll();
	protected:

	private:

	private slots:
		void changeCurrent(const QModelIndex& index, const QModelIndex& previous);
		void deleteIndex(const QModelIndex&);
		void deleteCurrent();

	private:
			void init();
};

#endif // _LEARNINGDIALOG_HXX
