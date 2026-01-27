/*
	QTDetection, bird voice visualization and comparison.
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

#include "DrawPlugin.hxx"

SignalDrawPlugin::SignalDrawPlugin(QObject *parent) : QObject(parent) {
	initialized = false;
}

bool SignalDrawPlugin::isInitialized() const{
	return initialized;
}

void SignalDrawPlugin::initialize(QDesignerFormEditorInterface *){
	if (initialized){
		return;
	}
	initialized = true;
}

QWidget* SignalDrawPlugin::createWidget(QWidget *parent){
	return new SignalDraw(parent);
}

QString SignalDrawPlugin::name() const {
	return "SignalDraw";
}

QString SignalDrawPlugin::group() const {
	return "Audio drawing";
}

QIcon SignalDrawPlugin::icon() const {
	return QIcon();
}

QString SignalDrawPlugin::toolTip() const {
	return "Signal...";
}

QString SignalDrawPlugin::whatsThis() const {
	return "";
}

bool SignalDrawPlugin::isContainer() const {
	return false;
}

QString SignalDrawPlugin::includeFile() const{
	return "SignalDraw.hxx";
}

QString SignalDrawPlugin::domXml() const {
	return "<widget class=\"SignalDraw\" name=\"signalDraw\">\n"
		" <property name=\"geometry\">\n"
		"  <rect>\n"
		"   <x>0</x>\n"
		"   <y>0</y>\n"
		"   <width>300</width>\n"
		"   <height>300</height>\n"
		"  </rect>\n"
		" </property>\n"
		" <property name=\"toolTip\" >\n"
		"  <string>Draws audio signal</string>\n"
		" </property>\n"
		" <property name=\"whatsThis\" >\n"
		"  <string>Draws signal.</string>\n"
		" </property>\n"
		"</widget>\n";
}

//SpectroDraw
SpectroDrawPlugin::SpectroDrawPlugin(QObject *parent) : QObject(parent) {
	initialized = false;
}

bool SpectroDrawPlugin::isInitialized() const{
	return initialized;
}

void SpectroDrawPlugin::initialize(QDesignerFormEditorInterface *){
	if (initialized){
		return;
	}
	initialized = true;
}

QWidget* SpectroDrawPlugin::createWidget(QWidget *parent){
	return new SpectroDraw(parent);
}

QString SpectroDrawPlugin::name() const {
	return "SpectroDraw";
}

QString SpectroDrawPlugin::group() const {
	return "Audio drawing";
}

QIcon SpectroDrawPlugin::icon() const {
	return QIcon();
}

QString SpectroDrawPlugin::toolTip() const {
	return "Spectrogram...";
}

QString SpectroDrawPlugin::whatsThis() const {
	return "";
}

bool SpectroDrawPlugin::isContainer() const {
	return false;
}

QString SpectroDrawPlugin::includeFile() const{
	return "SpectroDraw.hxx";
}

QString SpectroDrawPlugin::domXml() const {
	return "<widget class=\"SpectroDraw\" name=\"spectroDraw\">\n"
		" <property name=\"geometry\">\n"
		"  <rect>\n"
		"   <x>0</x>\n"
		"   <y>0</y>\n"
		"   <width>300</width>\n"
		"   <height>300</height>\n"
		"  </rect>\n"
		" </property>\n"
		" <property name=\"toolTip\" >\n"
		"  <string>Draws audio signal spectrogram</string>\n"
		" </property>\n"
		" <property name=\"whatsThis\" >\n"
		"  <string>Draws signal.</string>\n"
		" </property>\n"
		"</widget>\n";
}

//EnergyDraw
EnergyDrawPlugin::EnergyDrawPlugin(QObject *parent) : QObject(parent){
	initialized = false;
}
bool EnergyDrawPlugin::isContainer() const {
	return false;
}
bool EnergyDrawPlugin::isInitialized() const {
	return initialized;
}
QIcon EnergyDrawPlugin::icon() const {
	return QIcon();
}
QString EnergyDrawPlugin::domXml() const {
	return "<widget class=\"EnergyDrawWidget\" name=\"energyDrawWidget\">\n"
		" <property name=\"geometry\">\n"
		"  <rect>\n"
		"   <x>0</x>\n"
		"   <y>0</y>\n"
		"   <width>200</width>\n"
		"   <height>100</height>\n"
		"  </rect>\n"
		" </property>\n"
		" <property name=\"toolTip\" >\n"
		"  <string>Draws audio signal energy</string>\n"
		" </property>\n"
		" <property name=\"whatsThis\" >\n"
		"  <string>Draws signal's energy.</string>\n"
		" </property>\n"
		"</widget>\n";

}
QString EnergyDrawPlugin::group() const {
	return "Audio drawing";
}
QString EnergyDrawPlugin::includeFile() const {
	return "EnergyDrawWidget.hxx";
}
QString EnergyDrawPlugin::name() const {
	return "EnergyDrawWidget";
}
QString EnergyDrawPlugin::toolTip() const {
	return "";
}
QString EnergyDrawPlugin::whatsThis() const {
	return "";
}
QWidget* EnergyDrawPlugin::createWidget(QWidget *parent) {
	return new EnergyDrawWidget(parent);
}
void EnergyDrawPlugin::initialize(QDesignerFormEditorInterface *) {
	if (initialized){
		return;
	}
	initialized = true;
}

Q_EXPORT_PLUGIN(DrawingPlugins)
// Q_EXPORT_PLUGIN2(drawplugin, SpectroDrawPlugin)
// Q_EXPORT_PLUGIN2(drawplugin, SignalDrawPlugin)
