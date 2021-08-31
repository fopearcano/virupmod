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

#include "VIRUPSettings.hpp"

VIRUPSettings::VIRUPSettings(QWidget* parent)
    : SettingsWidget(parent)
{
	insertCustomGroup("data", tr("Data"), 0, new DataListWidget);

	insertGroup("simulation", tr("Simulation"), 1);
	addDateTimeSetting("starttime", QDateTime::currentDateTimeUtc(),
	                   tr("Start time (UTC)"));
	addBoolSetting("lockedrealtime", false, tr("Lock to Real Time"));

	addDirPathSetting("solarsystemdir", "solarsystem/systems/Solar System/",
	                  tr("Solar System Root Directory"));

	addDirPathSetting("planetsystemdir", "exoplanets/systems",
	                  tr("Exoplanetary Systems Root Directory"));

	insertGroup("misc", tr("Miscellaneous"), 3);
	addBoolSetting("showgrid", false, tr("Show Grid"));
	addColorSetting("gridcolor", QColor(255, 255, 255), tr("Grid Color"));
	addVector3DSetting("focuspoint", QVector3D(), tr("Focus Point"),
	                   {{tr("x"), tr("y"), tr("z")}}, -1000, 1000);
	// focuspoint=-0.352592, -0.062213, 0.144314
	addDoubleSetting("mintanangle", 0.05,
	                 tr("Minimum octree tan of open angle"));
	addUIntSetting("maxvramusagemb", 500, tr("Max VRAM Usage (in Mb)"), 0,
	               1000000);

	editGroup("graphics");
	addUIntSetting("texmaxsize", 4, tr("Textures max size (x2048)"), 1, 11);
	addUIntSetting("gentexload", 3, tr("Texture generation GPU load"), 1, 4);
	addUIntSetting("atmoquality", 6, tr("Atmosphere rendering quality"), 1, 5);
	addUIntSetting("maxlightcasters", 2,
	               tr("Maximum number of light casters per object"), 1, 2);
	addFilePathSetting("customfont", "", tr("Custom font file"));

	setCurrentIndex(0);
}

DataListWidget::DataListWidget()
{
	entries << tr("Cosmological Labels") << tr("CSV Stars")
	        << tr("CSV Galaxies") << tr("Cosmological Simulation")
	        << tr("Textured Sphere") << tr("Credits");
	entriesIds << "cosmolabels"
	           << "csvstars"
	           << "csvgalaxies"
	           << "cosmosim"
	           << "texsphere"
	           << "credits";

	loadMainLayout();
}

void DataListWidget::loadMainLayout()
{
	layout = new QVBoxLayout(this);
	layout->setSizeConstraint(QLayout::SetMinimumSize);

	auto w       = new QWidget(this);
	auto l       = new QHBoxLayout(w);
	auto label   = new QLabel(tr("Root directory :"));
	pathSelector = new PathSelector(this, tr("Data root directory"),
	                                PathSelector::Type::DIRECTORY);
	pathSelector->setPath(QSettings().value("data/rootdir").toString());
	connect(pathSelector, &PathSelector::pathChanged, this,
	        [](QString const& t) {
		        QSettings().setValue("data/rootdir", t + '/');
	        });
	l->addWidget(label);
	l->addWidget(pathSelector);
	layout->addWidget(w);

	w = new QWidget(this);
	w->resize(size());
	loadJsonRepresentation();

	w->setLayout(layout);
	setWidget(w);

	auto addButton = new QPushButton(this);
	addButton->setText(tr("Add..."));
	connect(addButton, &QPushButton::clicked, this, &DataListWidget::addData);
	layout->addWidget(addButton);

	w                 = new QWidget(this);
	auto hl           = new QHBoxLayout(w);
	auto importButton = new QPushButton(this);
	auto exportButton = new QPushButton(this);
	importButton->setText(tr("Import.."));
	exportButton->setText(tr("Export.."));
	connect(importButton, &QPushButton::clicked, this,
	        &DataListWidget::importJson);
	connect(exportButton, &QPushButton::clicked, this,
	        &DataListWidget::exportJson);
	hl->addWidget(importButton);
	hl->addWidget(exportButton);
	layout->addWidget(w);

	for(auto entry : dataJsonRepresentation["entries"].toArray())
	{
		addPushButtons(entry.toObject());
	}
}

void DataListWidget::loadJsonRepresentation()
{
	QJsonDocument jsondoc(QJsonDocument::fromJson(
	    QSettings().value("data/json").toString().toLatin1()));
	dataJsonRepresentation = jsondoc.object();
	if(dataJsonRepresentation.keys().indexOf("entries") == -1)
	{
		dataJsonRepresentation["entries"] = QJsonArray();
	}
}

void DataListWidget::saveJsonRepresentation()
{
	QJsonDocument jsondoc(dataJsonRepresentation);
	QSettings().setValue("data/json", QString(jsondoc.toJson()));
}

void DataListWidget::addData()
{
	DataDialog dd(entries, entriesIds);
	QJsonObject jsonEntry = dd.getDataDefinition();
	if(jsonEntry.keys().empty())
	{
		return;
	}

	addPushButtons(jsonEntry);

	auto array = dataJsonRepresentation["entries"].toArray();
	array.push_back(jsonEntry);
	dataJsonRepresentation["entries"] = array;
	saveJsonRepresentation();
}

void DataListWidget::addPushButtons(QJsonObject const& entry)
{
	auto w  = new QWidget(this);
	auto hl = new QHBoxLayout(w);
	w->setLayout(hl);

	auto b0 = new QPushButton(w);
	QString name(entry["name"].toString());
	QString type(entry["type"].toString());
	type = entries[entriesIds.indexOf(type)];
	b0->setText(name + "|" + type);
	hl->addWidget(b0);

	auto b1 = new QPushButton(w);
	b1->setText(tr("Remove"));
	b1->setMaximumWidth(150);
	hl->QLayout::addWidget(b1);

	int index(layout->count() - 2);
	if(index < 0)
	{
		index = 0;
	}
	layout->insertWidget(index, w);

	connect(b0, &QPushButton::clicked, [this, b0]() {
		QString name(b0->text().section('|', 0, 0));
		DataDialog dd(entries, entriesIds,
		              dataJsonRepresentation["entries"]
		                  .toArray()[getIndexInArray(name)]
		                  .toObject());
		QJsonObject jsonEntry = dd.getDataDefinition();
		if(jsonEntry.keys().empty())
		{
			return;
		}
		QString newName(jsonEntry["name"].toString());
		QString newType(jsonEntry["type"].toString());
		newType = entries[entriesIds.indexOf(newType)];
		b0->setText(newName + "|" + newType);
		updateEntry(name, jsonEntry);
	});

	connect(b1, &QPushButton::clicked, [this, b0, w]() {
		QString name(b0->text().section('|', 0, 0));
		if(QMessageBox::question(
		       this, tr("Removing data"),
		       tr("Do you really want to remove %1 ?").arg(name))
		   != QMessageBox::Yes)
		{
			return;
		}

		layout->removeWidget(w);
		w->hide();
		delete w;
		removeEntry(name);
	});
}

int DataListWidget::getIndexInArray(QString const& name) const
{
	for(int i(0); i < dataJsonRepresentation["entries"].toArray().size(); ++i)
	{
		if(dataJsonRepresentation["entries"]
		       .toArray()[i]
		       .toObject()["name"]
		       .toString()
		   == name)
		{
			return i;
		}
	}
	return -1;
}

void DataListWidget::updateEntry(QString const& name, QJsonObject const& entry)
{
	auto array(dataJsonRepresentation["entries"].toArray());
	array[getIndexInArray(name)]      = entry;
	dataJsonRepresentation["entries"] = array;
	saveJsonRepresentation();
}

void DataListWidget::removeEntry(QString const& name)
{
	auto array(dataJsonRepresentation["entries"].toArray());
	array.removeAt(getIndexInArray(name));
	dataJsonRepresentation["entries"] = array;
	saveJsonRepresentation();
}

void DataListWidget::exportJson()
{
	auto path(QFileDialog::getSaveFileName(this, tr("Export data profile"),
	                                       QDir::home().absolutePath(),
	                                       tr("JSON Files (*.json)")));
	QFile out(path);
	if(!out.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		return;
	}
	out.write(QJsonDocument(dataJsonRepresentation).toJson());
	out.close();
}

void DataListWidget::importJson()
{
	auto path(QFileDialog::getOpenFileName(this, tr("Import data profile"),
	                                       QDir::home().absolutePath(),
	                                       tr("JSON Files (*.json)")));

	QFile in(path);
	if(!in.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		return;
	}
	dataJsonRepresentation
	    = QJsonDocument(QJsonDocument::fromJson(in.readAll())).object();
	saveJsonRepresentation();
	delete layout;
	loadMainLayout();
	pathSelector->setPath(QFileInfo(in).absoluteDir().absolutePath());
}

DataDialog::DataDialog(QStringList const& entries,
                       QStringList const& entriesIds,
                       // NOLINTNEXTLINE(modernize-pass-by-value)
                       QJsonObject const& editFrom)
    : result(editFrom)
{
	layout = new QFormLayout;
	setLayout(layout);

	auto nameEdit = new QLineEdit;
	connect(nameEdit, &QLineEdit::textChanged,
	        [this](QString const& text) { this->result["name"] = text; });
	nameEdit->setText(result["name"].toString());
	layout->addRow(tr("Name :"), nameEdit);

	for(auto const& pair : UniverseElement::getLauncherFields(this, &result))
	{
		layout->addRow(pair.first, pair.second);
	}

	auto typeEdit = new QComboBox;
	for(auto const& entry : entries)
	{
		typeEdit->addItem(entry);
	}
	connect(typeEdit, &QComboBox::currentTextChanged,
	        [this, entries, entriesIds](QString const& text) {
		        this->result["type"] = entriesIds[entries.indexOf(text)];
	        });
	if(result.keys().indexOf("type") >= 0)
	{
		typeEdit->setCurrentText(
		    entries[entriesIds.indexOf(result["type"].toString())]);
	}
	else
	{
		result["type"] = entriesIds[0];
	}
	layout->addRow(tr("Type :"), typeEdit);

	auto w  = new QWidget(this);
	auto hl = new QHBoxLayout(w);
	w->setLayout(hl);
	auto accept = new QPushButton(this);
	accept->setText(tr("Accept"));
	connect(accept, &QPushButton::clicked, this, &QDialog::accept);
	auto cancel = new QPushButton(this);
	cancel->setText(tr("Cancel"));
	connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
	hl->addWidget(accept);
	hl->addWidget(cancel);
	layout->addRow("", w);

	setType(result["type"].toString());
	connect(typeEdit, &QComboBox::currentTextChanged,
	        [this, entries, entriesIds](QString const& text) {
		        setType(entriesIds[entries.indexOf(text)]);
	        });
}

QJsonObject DataDialog::getDataDefinition()
{
	auto ret = exec();
	if(!result["name"].toString().isEmpty() && ret == QDialog::Accepted)
	{
		return result;
	}
	return {};
}

void DataDialog::setType(QString const& type)
{
	if(specialized != nullptr)
	{
		layout->removeRow(layout->rowCount() - 2);
	}
	specialized     = new QWidget(this);
	auto speclayout = new QFormLayout(specialized);
	specialized->setLayout(speclayout);

	QList<QPair<QString, QWidget*>> fields;
	if(type == "cosmolabels")
	{
		fields = CosmologicalLabels::getLauncherFields(specialized, &result);
	}
	if(type == "csvstars")
	{
		fields = CSVObjects::getStarsLauncherFields(specialized, &result);
	}
	if(type == "csvgalaxies")
	{
		fields = CSVObjects::getGalaxiesLauncherFields(specialized, &result);
	}
	if(type == "cosmosim")
	{
		fields
		    = CosmologicalSimulation::getLauncherFields(specialized, &result);
	}
	if(type == "texsphere")
	{
		fields = TexturedSphere::getLauncherFields(specialized, &result);
	}
	if(type == "credits")
	{
		fields = Credits::getLauncherFields(specialized, &result);
	}
	for(auto const& pair : fields)
	{
		speclayout->addRow(pair.first, pair.second);
	}
	layout->insertRow(layout->rowCount() - 1, "", specialized);
}
