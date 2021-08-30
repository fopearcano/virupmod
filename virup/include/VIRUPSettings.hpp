/*
    Copyright (C) 2019 Florian Cabot <florian.cabot@epfl.ch>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef VIRUPSETTINGS_H
#define VIRUPSETTINGS_H

#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QScrollArea>
#include <QVBoxLayout>

#include "CSVObjects.hpp"
#include "CosmologicalLabels.hpp"
#include "CosmologicalSimulation.hpp"
#include "Credits.hpp"
#include "SettingsWidget.hpp"
#include "TexturedSphere.hpp"
#include "UniverseElement.hpp"

class VIRUPSettings : public SettingsWidget
{
  public:
	VIRUPSettings(QWidget* parent);
};

class DataListWidget : public QScrollArea
{
  public:
	DataListWidget();

  private:
	QJsonObject dataJsonRepresentation;
	QVBoxLayout* layout = nullptr;

	void loadMainLayout();
	void loadJsonRepresentation();
	void saveJsonRepresentation();
	void addData();
	void addPushButtons(QJsonObject const& entry);
	int getIndexInArray(QString const& name) const;
	void updateEntry(QString const& name, QJsonObject const& entry);
	void removeEntry(QString const& name);
	void exportJson();
	void importJson();

	QStringList entries;
	QStringList entriesIds;
};

class DataDialog : public QDialog
{
  public:
	DataDialog(QStringList const& entries, QStringList const& entriesIds,
	           QJsonObject const& editFrom = {});
	QJsonObject getDataDefinition();

  private:
	void setType(QString const& type);

	QJsonObject result;

	QFormLayout* layout;
	QWidget* specialized = nullptr;
};

#endif // VIRUPSETTINGS_H
