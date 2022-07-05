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

#include <QNetworkAccessManager>
#include <QNetworkReply>

VIRUPSettings::VIRUPSettings(QWidget* parent)
    : SettingsWidget(parent)
{
	auto dlw = new DataListWidget;
	insertCustomGroup("data", tr("Data"), 0, dlw);

	insertGroup("simulation", tr("Simulation"), 1);
	addDateTimeSetting("starttime", QDateTime::currentDateTimeUtc(),
	                   tr("Start time (UTC)"));
	addBoolSetting("lockedrealtime", false, tr("Lock to Real Time"));

	addDirPathSetting("solarsystemdir", "solarsystem/systems/Solar System/",
	                  tr("Solar System Root Directory"));

	addDirPathSetting("planetsystemdir", "exoplanets/systems",
	                  tr("Exoplanetary Systems Root Directory"));

	insertGroup("misc", tr("Miscellaneous"), 3);
	addColorSetting("uilabelscolor", QColor(255, 165, 0),
	                tr("UI Labels Color"));
	addFilePathSetting("uilabelsfont", "", tr("UI Labels Font file"));
	addDoubleSetting("uilabelsdistmul", 1.0, tr("UI Labels Distance Mult."));
	addBoolSetting("halfcavehelper", false, tr("Show Half-Cave helper"));
	addBoolSetting("showgrid", false, tr("Show Grid"));
	addColorSetting("gridcolor", QColor(255, 255, 255), tr("Grid Color"));
	addDoubleSetting("mintanangle", 0.05,
	                 tr("Minimum octree tan of open angle"));
	addUIntSetting("maxvramusagemb", 500, tr("Max VRAM Usage (in Mb)"), 0,
	               1000000);
	addDirPathSetting("octreestatesdir", QDir::homePath() + "/octree_states/",
	                  tr("Octree states (OBJ) save directory"));

	editGroup("graphics");
	addUIntSetting("texmaxsize", 8, tr("Textures max size (x2048)"), 1, 11);
	addUIntSetting("gentexload", 1, tr("Texture generation GPU load"), 1, 4);
	addUIntSetting("atmoquality", 1, tr("Atmosphere rendering quality"), 1, 5);
	addUIntSetting("maxlightcasters", 1,
	               tr("Maximum number of light casters per object"), 1, 2);
	addFilePathSetting("customfont", "", tr("Custom font file"));

	setCurrentIndex(0);

	// DOWNLOADER
	if(!QSettings().value("data/rootdir").toString().isEmpty())
	{
		return;
	}
	auto yesOrNo = QMessageBox::question(
	    this, tr("Data Download"),
	    tr("No data detected. Do you want to download the default data (3.5GB "
	       "download/7.1GB uncompressed) ? (Data is required to visualize "
	       "anything, press No if you already have some data to visualize on "
	       "hand.)"));

	if(yesOrNo != QMessageBox::StandardButton::Yes)
	{
		return;
	}

	bool ok(false);
	QString downloadDir;
	do
	{
		downloadDir = QFileDialog::getExistingDirectory(
		    this, tr("Select a data storage directory (>= 10.7GB capacity)"),
		    "/media/florian/Archive");
		if(downloadDir.isEmpty())
		{
			return;
		}
		QStorageInfo info(downloadDir);
		if(info.bytesAvailable() <= 10.7 * 1024 * 1024 * 1024) // 10.7GB
		{
			QString avail
			    = QString::number(info.bytesAvailable() / 1024.0 / 1024 / 1024);
			QMessageBox::warning(this, tr("Not enough storage space"),
			                     avail + tr(" GB available, 10.7GB needed."));
		}
		else
		{
			ok = true;
		}
	} while(!ok);

	QFile downloadedFile(downloadDir + "/VIRUP-DATA.zip");
	downloadedFile.open(QIODevice::WriteOnly);

	QUrl url("ftp://obsftp.unige.ch/pub/cabot/VIRUP-DATA.zip");

	QNetworkAccessManager nam;
	auto rep = nam.get(QNetworkRequest(url));

	QProgressDialog progress;
	progress.setWindowTitle(tr("Downloading..."));
	progress.show();

	QElapsedTimer timer;
	timer.start();

	qint64 downloaded(0);

	connect(rep, &QNetworkReply::downloadProgress,
	        [&downloadedFile, rep, &progress, &timer, &downloaded](qint64 recv,
	                                                               qint64 tot) {
		        QByteArray b = rep->readAll();
		        downloadedFile.write(b);
		        progress.setMaximum(tot / 1024 / 1024);
		        progress.setValue(recv / 1024 / 1024);
		        auto dt              = timer.restart() / 1000.0;
		        auto speedInMBPerSec = b.size() / dt / (1024 * 1024);
		        downloaded += b.size();
		        int remaining = static_cast<int>(round(
		            (tot - downloaded) / (speedInMBPerSec * 1024 * 1024)));
		        progress.setLabelText(
		            QString::number(recv / 1024.0 / 1024 / 1024, 'g', 2) + "GB/"
		            + QString::number(tot / 1024.0 / 1024 / 1024, 'g', 2)
		            + "GB " + QString::number(speedInMBPerSec, 'g', 3)
		            + " MB/s ETA: "
		            + QTime(0, 0).addSecs(remaining).toString("hh:mm:ss")
		            + " secs");
	        });

	bool keepDownloading = true;
	connect(&progress, &QProgressDialog::canceled,
	        [&keepDownloading]() { keepDownloading = false; });

	while(rep->isRunning() && keepDownloading)
	{
		QCoreApplication::processEvents();
		QThread::usleep(100000);
	}
	rep->deleteLater();

	if(!keepDownloading)
	{
		return;
	}

	progress.setWindowTitle(tr("Extracting..."));
	progress.setLabelText(tr("Waiting for data archive to be extracted..."));
	progress.setMaximum(9);
	progress.show();
	QProcess unzipProcess;
#ifdef Q_OS_WIN
	QString cmd("powershell -command \"Expand-Archive ");
	cmd += downloadDir + "\\VIRUP-DATA.zip ";
	cmd += downloadDir + "\"";
	unzipProcess.start(cmd);
#else
	QString cmd("unzip ");
	cmd += downloadDir + "/VIRUP-DATA.zip -d ";
	cmd += downloadDir;
	unzipProcess.start(cmd, QStringList{});
#endif
	while(unzipProcess.state() != QProcess::NotRunning)
	{
		QDir d(downloadDir + "/VIRUP-DATA/");
		d.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
		progress.setValue(d.count() - 2);
		QCoreApplication::processEvents();
		QThread::usleep(100000);
	}

	dlw->importJsonFromPath(downloadDir + "/VIRUP-DATA/vrdemo-cmb.json");
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
	importJsonFromPath(path);
}

void DataListWidget::importJsonFromPath(QString const& path)
{
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
