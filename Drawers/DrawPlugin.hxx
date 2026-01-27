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

#ifndef _DRAWPLUGIN_HXX
#define _DRAWPLUGIN_HXX

#include <QObject>
#include <QtPlugin>
#include <QDesignerCustomWidgetInterface>
#include "SignalDraw.hxx"
#include "SpectroDraw.hxx"
#include "EnergyDrawWidget.hxx"

class SignalDrawPlugin : public QObject, public QDesignerCustomWidgetInterface
{
	Q_OBJECT
		Q_INTERFACES(QDesignerCustomWidgetInterface)

	public:
		SignalDrawPlugin(QObject *parent = 0);

		bool isContainer() const;
		bool isInitialized() const;
		QIcon icon() const;
		QString domXml() const;
		QString group() const;
		QString includeFile() const;
		QString name() const;
		QString toolTip() const;
		QString whatsThis() const;
		QWidget *createWidget(QWidget *parent);
		void initialize(QDesignerFormEditorInterface *core);

	private:
		bool initialized;
};

class SpectroDrawPlugin : public QObject, public QDesignerCustomWidgetInterface
{
	Q_OBJECT
		Q_INTERFACES(QDesignerCustomWidgetInterface)

	public:
		SpectroDrawPlugin(QObject *parent = 0);

		bool isContainer() const;
		bool isInitialized() const;
		QIcon icon() const;
		QString domXml() const;
		QString group() const;
		QString includeFile() const;
		QString name() const;
		QString toolTip() const;
		QString whatsThis() const;
		QWidget *createWidget(QWidget *parent);
		void initialize(QDesignerFormEditorInterface *core);

	private:
		bool initialized;
};

class EnergyDrawPlugin : public QObject, public QDesignerCustomWidgetInterface
{
	Q_OBJECT
		Q_INTERFACES(QDesignerCustomWidgetInterface)
	public:
		EnergyDrawPlugin(QObject *parent = 0);
		bool isContainer() const;
		bool isInitialized() const;
		QIcon icon() const;
		QString domXml() const;
		QString group() const;
		QString includeFile() const;
		QString name() const;
		QString toolTip() const;
		QString whatsThis() const;
		QWidget *createWidget(QWidget *parent);
		void initialize(QDesignerFormEditorInterface *);

	private:
		bool initialized;
};

class DrawingPlugins : public QObject, public QDesignerCustomWidgetCollectionInterface
{
	Q_OBJECT
		Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)
	public:
		QList<QDesignerCustomWidgetInterface*> customWidgets() const
		{
			QList<QDesignerCustomWidgetInterface *> plugins;
			return plugins << new SpectroDrawPlugin << new SignalDrawPlugin << new EnergyDrawPlugin;
		}
};

#endif// _DRAWPLUGIN_HXX
