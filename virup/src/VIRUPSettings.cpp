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

#include <QProgressDialog>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QtConcurrent>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "Zip.hpp"
#include "gui/PathSelector.hpp"
#include "network/DownloadManager.hpp"
#include "universe/CSVObjects.hpp"
#include "universe/CosmologicalLabels.hpp"
#include "universe/CosmologicalSimulation.hpp"
#include "universe/Credits.hpp"
#include "universe/ImageCatalog.hpp"
#include "universe/TexturedSphere.hpp"

#include "LibPlanet.hpp"
#include "LibTerrain.hpp"

VIRUPSettings::VIRUPSettings(QWidget* parent)
    : SettingsWidget(parent)
{
	trn::LibTerrain libterrain;
	libterrain.setupSettings(*this);

	LibPlanet libplanet;
	libplanet.setupSettings(*this);

	editGroup("simulation", true);

	addDirPathSetting("solarsystemdir", "solarsystem/systems/Solar System/",
	                  tr("Solar System Root Directory"));

	addDirPathSetting("planetsystemdir", "exoplanets/systems",
	                  tr("Exoplanetary Systems Root Directory"));

	auto* dlw = make_qt_unique<DataListWidget>(*this);
	insertCustomGroup("data", tr("Data"), 0, dlw);

	insertGroup("misc", tr("Miscellaneous"), 3);
	addDoubleSetting("disttoorigin", 1.5,
	                 tr("Default object distance to origin (in m)"));
	addColorSetting("uilabelscolor", QColor(255, 165, 0),
	                tr("UI Labels Color"));
	addFilePathSetting("uilabelsfont", "", tr("UI Labels Font file"));
	addVector3DSetting("uilabelspos", {-0.3f, 0.25f, -0.4f},
	                   tr("UI Labels Position (cam space)"), {"x", "y", "z"},
	                   -5.f, 5.f);
	addDoubleSetting("uilabelssizemul", 1.0, tr("UI Labels Size Mult."));
	addDoubleSetting("uilabelsdistmul", 1.0, tr("UI Labels Distance Mult."));
	addBoolSetting("halfcavehelper", false, tr("Show Half-Cave helper"));
	addBoolSetting("idlemode", false, tr("Allow Idle Mode"));
	addDoubleSetting("idlemodewaittime", 60.0,
	                 tr("Time to trigger idle mode (in s)"));
	addBoolSetting("showgrid", false, tr("Show Grid"));
	addColorSetting("gridcolor", QColor(255, 255, 255), tr("Grid Color"));
	addDoubleSetting("mintanangle", 0.05,
	                 tr("Minimum octree tan of open angle"));
	addUIntSetting("maxvramusagemb", 500, tr("Max VRAM Usage (in Mb)"), 0,
	               1000000);
	addDirPathSetting("octreestatesdir", QDir::homePath() + "/octree_states/",
	                  tr("Octree states (OBJ) save directory"));
	addScreenSetting("presenterscreen", "", tr("Presenter screen"));

	insertGroup("sound", tr("Sound"), 4);
	addDoubleSetting("ambiancevolume", 0.0,
	                 tr("Ambiant music volume (0.0-1.0)"), 0.0, 1.0);
	addDoubleSetting("controllervolume", 0.0,
	                 tr("Controller sounds volume (0.0-1.0)"), 0.0, 1.0);

	editGroup("graphics", true);
	addFilePathSetting("customfont", "", tr("Custom font file"));

	editGroup("controls");
	addDoubleSetting("translationspeed", 1.0,
	                 tr("Gamepad Translation Speed Multiplier"));
	addDoubleSetting("rotationspeed", 1.0,
	                 tr("Gamepad Rotation Speed Multiplier"));

	setCurrentIndex(0);

	// DOWNLOADER
	if(!QSettings().value("data/rootdir").toString().isEmpty())
	{
		return;
	}
	dlw->downloadDefaultData();
}

DataListWidget::DataListWidget(QWidget* parent)
{
	QObject::setParent(
	    parent); // messes up layout if put in QScrollArea constructor
	entries << tr("Cosmological Labels") << tr("CSV Stars")
	        << tr("CSV Galaxies") << tr("Cosmological Simulation")
	        << tr("Textured Sphere") << tr("Credits") << tr("Image Catalog");
	entriesIds << "cosmolabels" << "csvstars" << "csvgalaxies" << "cosmosim"
	           << "texsphere" << "credits" << "imgcatalog";

	loadMainLayout();
}

void DataListWidget::importJsonFromPath(QString const& path)
{
	QFile in(path);
	if(!in.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		return;
	}
	dataJsonRepresentation = QJsonDocument::fromJson(in.readAll()).object();
	saveJsonRepresentation();
	delete layout;
	loadMainLayout();
	pathSelector->setPath(QFileInfo(in).absoluteDir().absolutePath());
}

void DataListWidget::downloadDefaultData()
{
	auto yesOrNo = QMessageBox::question(
	    this, tr("Data Download"),
	    tr("No data detected. Do you want to download the default data "
	       "(11.2GiB download/15.9GiB uncompressed) ? (Data is required to "
	       "visualize anything, press No if you already have some data to "
	       "visualize on hand.)"));

	if(yesOrNo != QMessageBox::StandardButton::Yes)
	{
		return;
	}

	bool ok(false);
	QString downloadDir;
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-do-while)
	do
	{
		downloadDir = QFileDialog::getExistingDirectory(
		    this, tr("Select a data storage directory (>= 27.1GiB capacity)"),
		    QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0]);
		if(downloadDir.isEmpty())
		{
			return;
		}
		const QStorageInfo info(downloadDir);
		qint64 alreadyDownloaded = 0;
		if(QFile(downloadDir + "/VIRUP-DATA.zip").exists())
		{
			alreadyDownloaded = QFile(downloadDir + "/VIRUP-DATA.zip").size();
		}
		if(info.bytesAvailable() / 1024.0
		   <= 27.1 * 1024 * 1024 - (alreadyDownloaded / 1024.0)) // 27.1GiB
		{
			const QString avail
			    = QString::number(info.bytesAvailable() / 1024.0 / 1024 / 1024);
			QMessageBox::warning(this, tr("Not enough storage space"),
			                     avail + tr(" GiB available, 27.1GiB needed."));
		}
		else
		{
			ok = true;
		}
	} while(!ok);

	auto downloadedFilePath = downloadDir + "/VIRUP-DATA.zip";
	const QUrl url("https://www.astro.unige.ch/~cabot/VIRUP-DATA.zip");
	auto totsize = DownloadManager::getFileSize(url);
	QFile downloadedFile(downloadedFilePath);
	DownloadManager::downloadFileSync(url, downloadedFile,
	                                  {.showProgress = true}, this);

	auto fileSize(downloadedFile.size());
	//  if download has somewhat failed
	if(fileSize != totsize)
	{
		QMessageBox::warning(
		    this, tr("Download failed"),
		    tr("Download failed: You can retry later by launching VIRUP again. "
		       "Download will resume to where it was (if you "
		       "choose the same directory)."));
		return;
	}

	if(!Zip::decompress(downloadedFilePath, downloadDir, true, this))
	{
		qDebug() << "Cancelled";
	}

	downloadedFile.remove();
	importJsonFromPath(downloadDir + "/VIRUP-DATA/vrdemo.json");
}

void DataListWidget::loadMainLayout()
{
	layout = make_qt_unique<QVBoxLayout>(*this);
	layout->setSizeConstraint(QLayout::SetMinimumSize);

	auto* downloadButton
	    = make_qt_unique<QPushButton>(*this, tr("Download default data..."));
	connect(downloadButton, &QPushButton::pressed,
	        [this]() { downloadDefaultData(); });
	layout->addWidget(downloadButton);

	auto* w      = make_qt_unique<QWidget>(*this);
	auto* l      = make_qt_unique<QHBoxLayout>(*w);
	auto* label  = make_qt_unique<QLabel>(*w, tr("Root directory :"));
	pathSelector = make_qt_unique<PathSelector>(
	    *this, tr("Data root directory"), PathSelector::Type::DIRECTORY);
	pathSelector->setPath(QSettings().value("data/rootdir").toString());
	connect(pathSelector, &PathSelector::pathChanged, this, [](QString const& t)
	        { QSettings().setValue("data/rootdir", t + '/'); });
	l->addWidget(label);
	l->addWidget(pathSelector);
	layout->addWidget(w);

	w = make_qt_unique<QWidget>(*this);
	w->resize(size());
	loadJsonRepresentation();

	w->setLayout(layout);
	setWidget(w);

	auto* addButton = make_qt_unique<QPushButton>(*this);
	addButton->setText(tr("Add..."));
	connect(addButton, &QPushButton::clicked, this, &DataListWidget::addData);
	layout->addWidget(addButton);

	w                  = make_qt_unique<QWidget>(*this);
	auto* hl           = make_qt_unique<QHBoxLayout>(*w);
	auto* importButton = make_qt_unique<QPushButton>(*this);
	auto* exportButton = make_qt_unique<QPushButton>(*this);
	importButton->setText(tr("Import.."));
	exportButton->setText(tr("Export.."));
	connect(importButton, &QPushButton::clicked, this,
	        &DataListWidget::importJson);
	connect(exportButton, &QPushButton::clicked, this,
	        &DataListWidget::exportJson);
	hl->addWidget(importButton);
	hl->addWidget(exportButton);
	layout->addWidget(w);
	layout->addStretch();

	for(auto entry : dataJsonRepresentation["entries"].toArray())
	{
		addPushButtons(entry.toObject());
	}
}

void DataListWidget::loadJsonRepresentation()
{
	const QJsonDocument jsondoc(QJsonDocument::fromJson(
	    QSettings().value("data/json").toString().toLatin1()));
	dataJsonRepresentation = jsondoc.object();
	if(dataJsonRepresentation.keys().indexOf("entries") == -1)
	{
		dataJsonRepresentation["entries"] = QJsonArray();
	}
}

void DataListWidget::saveJsonRepresentation()
{
	const QJsonDocument jsondoc(dataJsonRepresentation);
	QSettings().setValue("data/json", QString(jsondoc.toJson()));
}

void DataListWidget::addData()
{
	DataDialog dd(entries, entriesIds);
	const QJsonObject jsonEntry = dd.getDataDefinition();
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
	const QString name(entry["name"].toString());
	QString type(entry["type"].toString());
	if(entriesIds.indexOf(type) == -1)
	{
		qWarning() << "Can't add object" << name << "of type" << type
		           << "Inknown type.";
		return;
	}

	auto* w  = make_qt_unique<QWidget>(*this);
	auto* hl = make_qt_unique<QHBoxLayout>(*w);
	w->setLayout(hl);

	auto* b0 = make_qt_unique<QPushButton>(*w);
	type     = entries[entriesIds.indexOf(type)];
	b0->setText(name + "|" + type);
	hl->addWidget(b0);

	auto* b1 = make_qt_unique<QPushButton>(*w);
	b1->setText(tr("Remove"));
	b1->setMaximumWidth(150);
	hl->QLayout::addWidget(b1);

	int index(layout->count() - 2);
	if(index < 0)
	{
		index = 0;
	}
	layout->insertWidget(index, w);

	connect(b0, &QPushButton::clicked,
	        [this, b0]()
	        {
		        const QString name(b0->text().section('|', 0, 0));
		        DataDialog dd(entries, entriesIds,
		                      dataJsonRepresentation["entries"]
		                          .toArray()[getIndexInArray(name)]
		                          .toObject());
		        QJsonObject jsonEntry = dd.getDataDefinition();
		        if(jsonEntry.keys().empty())
		        {
			        return;
		        }
		        const QString newName(jsonEntry["name"].toString());
		        QString newType(jsonEntry["type"].toString());
		        newType = entries[entriesIds.indexOf(newType)];
		        b0->setText(newName + "|" + newType);
		        updateEntry(name, jsonEntry);
	        });

	connect(b1, &QPushButton::clicked,
	        [this, b0, w]()
	        {
		        const QString name(b0->text().section('|', 0, 0));
		        if(QMessageBox::question(
		               this, tr("Removing data"),
		               tr("Do you really want to remove %1 ?").arg(name))
		           != QMessageBox::Yes)
		        {
			        return;
		        }

		        layout->removeWidget(w);
		        w->hide();
		        w->deleteLater();
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
	importJsonFromPath(path);
}

DataDialog::DataDialog(QStringList const& entries,
                       QStringList const& entriesIds,
                       // NOLINTNEXTLINE(modernize-pass-by-value)
                       QJsonObject const& editFrom)
    : result(editFrom)
    , layout(make_qt_unique<QFormLayout>(*this))
{
	setLayout(layout);

	auto* nameEdit = make_qt_unique<QLineEdit>(*this);
	connect(nameEdit, &QLineEdit::textChanged,
	        [this](QString const& text) { this->result["name"] = text; });
	nameEdit->setText(result["name"].toString());
	layout->addRow(tr("Name :"), nameEdit);

	for(auto const& pair : UniverseElement::getLauncherFields(*this, result))
	{
		layout->addRow(pair.first, pair.second);
	}

	auto* typeEdit = make_qt_unique<QComboBox>(*this);
	for(auto const& entry : entries)
	{
		typeEdit->addItem(entry);
	}
	connect(typeEdit, &QComboBox::currentTextChanged,
	        [this, entries, entriesIds](QString const& text)
	        { this->result["type"] = entriesIds[entries.indexOf(text)]; });
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

	auto* w  = make_qt_unique<QWidget>(*this);
	auto* hl = make_qt_unique<QHBoxLayout>(*w);
	w->setLayout(hl);
	auto* accept = make_qt_unique<QPushButton>(*this);
	accept->setText(tr("Accept"));
	connect(accept, &QPushButton::clicked, this, &QDialog::accept);
	auto* cancel = make_qt_unique<QPushButton>(*this);
	cancel->setText(tr("Cancel"));
	connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
	hl->addWidget(accept);
	hl->addWidget(cancel);
	layout->addRow("", w);

	setType(result["type"].toString());
	connect(typeEdit, &QComboBox::currentTextChanged,
	        [this, entries, entriesIds](QString const& text)
	        { setType(entriesIds[entries.indexOf(text)]); });
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
	specialized      = make_qt_unique<QWidget>(*this);
	auto* speclayout = make_qt_unique<QFormLayout>(*specialized);
	specialized->setLayout(speclayout);

	QList<QPair<QString, QWidget*>> fields;
	if(type == "cosmolabels")
	{
		fields = CosmologicalLabels::getLauncherFields(*specialized, result);
	}
	if(type == "csvstars")
	{
		fields = CSVObjects::getStarsLauncherFields(*specialized, result);
	}
	if(type == "csvgalaxies")
	{
		fields = CSVObjects::getGalaxiesLauncherFields(*specialized, result);
	}
	if(type == "cosmosim")
	{
		fields
		    = CosmologicalSimulation::getLauncherFields(*specialized, result);
	}
	if(type == "texsphere")
	{
		fields = TexturedSphere::getLauncherFields(*specialized, result);
	}
	if(type == "credits")
	{
		fields = Credits::getLauncherFields(*specialized, result);
	}
	if(type == "imgcatalog")
	{
		fields = ImageCatalog::getLauncherFields(*specialized, result);
	}
	for(auto const& pair : fields)
	{
		speclayout->addRow(pair.first, pair.second);
	}
	layout->insertRow(layout->rowCount() - 1, "", specialized);
}
