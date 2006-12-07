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

void LearningDialog::init(){
	setupUi(this);
}

LearningDialog::~LearningDialog(){
};

void LearningDialog::connectAll(){
	connect( learningList->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(changeCurrent(const QModelIndex&, const QModelIndex&)) );
	// connect( learningList, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(deleteIndex(const QModelIndex&)) );
	connect( learningList, SIGNAL(backSpacePressed()), this, SLOT(deleteCurrent()) );
}

void LearningDialog::changeCurrent(const QModelIndex& index, const QModelIndex& /*previous*/){
	if (!index.isValid()){
		return;
	}
	QVariant q = index.data(9999);
	CSample* sample = (CSample*)q.value<int>();
	spectroDraw->setSample(sample, false);
}

void LearningDialog::deleteIndex(const QModelIndex& index){
	learningList->model()->removeRow(index.row());
}

void LearningDialog::deleteCurrent(){
	QModelIndex index = learningList->currentIndex();
	learningList->model()->removeRow(index.row());
}
