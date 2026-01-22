/*
 * =====================================================================================
 * 
 *        Filename:  SettingsDialog.cpp
 * 
 *     Description:  
 * 
 *         Version:  1.0
 *         Created:  2006-06-15 08:17:23 CEST
 *        Revision:  none
 *        Compiler:  gcc
 * 
 *          Author:   Roman Kamyk
 *         Company:  
 * 
 * =====================================================================================
 */

#include "LearningDialog.hxx"
#include "MyListView.hxx"
#include <QItemSelectionModel>

void LearningDialog::init(){
	setupUi(this);
}

LearningDialog::~LearningDialog(){
};

void LearningDialog::connectAll(){
	connect(learningList->selectionModel(), &QItemSelectionModel::currentChanged, this, &LearningDialog::changeCurrent);
	connect(learningList, &CMyListView::backSpacePressed, this, &LearningDialog::deleteCurrent);
}

void LearningDialog::changeCurrent(const QModelIndex& index, const QModelIndex& /*previous*/){
	if (!index.isValid()){
		return;
	}
	QVariant q = index.data(9999);
	CSample* sample = reinterpret_cast<CSample*>(q.value<quintptr>());
	spectroDraw->setSample(sample, false);
}

void LearningDialog::deleteIndex(const QModelIndex& index){
	learningList->model()->removeRow(index.row());
}

void LearningDialog::deleteCurrent(){
	QModelIndex index = learningList->currentIndex();
	learningList->model()->removeRow(index.row());
}
