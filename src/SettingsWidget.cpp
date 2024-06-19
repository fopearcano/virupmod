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

#include "SettingsWidget.hpp"

#include "GamepadHandler.hpp"

#include <QStandardPaths>

SettingsWidget::SettingsWidget(QWidget* parent)
    : QTabWidget(parent)
{
	qDebug() << QString("Config file :") + QSettings().fileName();

	// update settings file if needed
	if(!QSettings().contains("window/windefinitions")
	   && QSettings().contains("window/width"))
	{
		QStringList windef;
		RenderingWindow::Parameters p;
		p.width      = QSettings().value("window/width").toUInt();
		p.height     = QSettings().value("window/height").toUInt();
		p.fullscreen = QSettings().value("window/fullscreen").toBool();
		p.screenname = QSettings().value("window/screenname").toString();
		p.horizontalAngleShift
		    = QSettings().value("network/angleshift").toDouble();
		p.verticalAngleShift
		    = QSettings().value("network/vangleshift").toDouble();

		QSettings().remove("window/width");
		QSettings().remove("window/height");
		QSettings().remove("window/fullscreen");
		QSettings().remove("window/screenname");
		QSettings().remove("network/angleshift");
		QSettings().remove("network/vangleshift");
		QSettings().remove("vr/forceleft");
		QSettings().remove("vr/forceright");
		windef << p.toStr();
		QSettings().setValue("window/windefinitions", windef);
	}
	// end update

	addGroup("window", tr("Window"));
	addWindowsDefinitionSettings();
	addBoolSetting("vsync", false, tr("Enable VSYNC"));
	addBoolSetting("forcerenderresolution", false,
	               tr("Force Rendering Resolution"));
	addUIntSetting("forcewidth", 1500, tr("Forced Rendering Width"), 0, 17000);
	addUIntSetting("forceheight", 800, tr("Forced Rendering Height"), 0, 17000);
	addStringAmongListSetting("projection",
	                          {"default", "panorama360", "vr180l", "vr180r",
	                           "vr180", "domemaster180"},
	                          {tr("Default"), tr("Panorama 360"),
	                           tr("VR 180 Left"), tr("VR 180 Right"),
	                           tr("VR 180"), tr("Domemaster 180")},
	                          tr("Projection"));
	addLanguageSetting();
	addBoolSetting("videomode", false, tr("Start with video mode enabled"));
	addUIntSetting("maxframe", 0,
	               tr("Quit after this number of frames rendered"), 0, 1000000);
	addUIntSetting("videofps", 60, tr("Video target FPS"), 0, 500);
	addDirPathSetting(
	    "viddir",
	    QStandardPaths::standardLocations(QStandardPaths::MoviesLocation)[0]
	        + '/' + PROJECT_NAME,
	    tr("Video Frames Output Directory"));

	addGroup("graphics", tr("Graphics"));
	addUIntSetting("antialiasing", 0, tr("Anti-aliasing"), 0, 3);
	addUIntSetting("shadowsquality", 1, tr("Shadows Quality"), 1, 5);
	addUIntSetting("smoothshadows", 0, tr("Shadow Smoothing Quality"), 0, 5);
	addBoolSetting("dithering", false, tr("Enable Dithering"));
	addBoolSetting("bloom", false, tr("Bloom"));
	addDoubleSetting("vfov", 0.0, tr("Vertical field of view (0=auto)"), 0.0,
	                 360.0);
	addDoubleSetting("hfov", 0.0, tr("Horizontal field of view (0=auto)"), 0.0,
	                 360.0);

	InputManager inputManager;
	addGroup("controls", tr("Controls"));
	currentForm->addRow(tr("ENGINE"), make_qt_unique<QWidget>(*this));

	QStringList vals   = {"-1"};
	QStringList labels = {tr("None")};
	unsigned int defaultIndex(0);
	for(auto const& pair : GamepadHandler::getConnectedGamepads(true))
	{
		vals << QString::number(pair.first);
		labels << pair.second;
		defaultIndex = 1;
	}
	auto comboBox = addStringAmongListSetting("gamepad", vals, labels,
	                                          tr("Gamepad"), defaultIndex);

	auto refresh = make_qt_unique<QPushButton>(*this, tr("Refresh"));
	connect(refresh, &QPushButton::pressed,
	        [this, comboBox]()
	        {
		        QStringList vals   = {"-1"};
		        QStringList labels = {tr("None")};
		        for(auto const& pair :
		            GamepadHandler::getConnectedGamepads(true))
		        {
			        vals << QString::number(pair.first);
			        labels << pair.second;
		        }

		        QString currentVal(
		            settings.value("controls/gamepad", "-1").toString());
		        int currentIndex(vals.indexOf(currentVal));

		        comboBox->clear();
		        for(int i(0); i < labels.size(); ++i)
		        {
			        auto const& label(labels[i]);
			        auto const& val(vals[i]);
			        comboBox->addItem(label, val);
		        }
		        comboBox->setCurrentIndex(currentIndex);
	        });
	currentForm->addRow("", refresh);

	for(auto const& key : inputManager.getOrderedEngineKeys())
	{
		addKeySequenceSetting(inputManager[key].id, key,
		                      inputManager[key].name);
	}
	if(!inputManager.getOrderedProgramKeys().empty())
	{
		currentForm->addRow(" ", make_qt_unique<QWidget>(*this));
		currentForm->addRow(PROJECT_NAME, make_qt_unique<QWidget>(*this));
		for(auto const& key : inputManager.getOrderedProgramKeys())
		{
			addKeySequenceSetting(inputManager[key].id, key,
			                      inputManager[key].name);
		}
	}

	addGroup("vr", tr("Virtual Reality"));
	addBoolSetting("enabled", true, tr("Enable VR"));
	addStringAmongListSetting("handler", {"openvr", "stereobeamer"},
	                          {tr("OpenVR"), tr("Stereo Beamer")},
	                          tr("Handler"));
	addBoolSetting(
	    "thirdrender", false,
	    tr("Force 2D render on screen\n(will decrease performance !)"));
	addDoubleSetting("stereomultiplier", 1.0,
	                 tr("Stereo multiplier (if applicable)"), 0.0, 1000.0);
	addVector3DSetting("virtualcamshift", {},
	                   "Virtual Camera Shift\n(1.0 = screen physical height)",
	                   {"x", "y", "z"}, 0.f, 100.f);
	addDoubleSetting("screenheight", 2.0, tr("Screen height (in meters)"));

	addGroup("network", tr("Network"));
	addBoolSetting("server", true, tr("Server"));
	addUIntSetting("clientid", 0, tr("Client ID"));
	addStringSetting("ip", "127.0.0.1", tr("IP address of server (if client)"));
	addUIntSetting("port", 5000, tr("UDP port"), 1025, 49999);
	addUIntSetting("tcpport", 5001, tr("TCP port"), 1025, 49999);

	QString scriptsDir("./data/" + QString(PROJECT_DIRECTORY) + "/scripts/");
	QDirIterator it(scriptsDir, QStringList() << "main.py", QDir::Files,
	                QDirIterator::Subdirectories);
	QStringList dirList;
	while(it.hasNext())
	{
		QString path(it.next());
		path.replace(scriptsDir, "");
		path.replace("/main.py", "");
		dirList << path;
	}
	dirList.sort();
	dirList << "";
	addGroup("scripting", tr("Scripting"));
	addStringAmongListSetting("rootdir", dirList, dirList,
	                          tr("Scripts Root Directory"));
	addDirPathSetting("customdir", "", tr("Custom Root Directory"));

	addGroup("debugcamera", tr("Debug Camera"));
	addBoolSetting("enabled", false, tr("Enable Debug Camera"));
	addBoolSetting("followhmd", false, tr("Follow HMD Movement"));
	addBoolSetting("debuginheadset", false, tr("Show Debug In HMD"));
}

void SettingsWidget::addGroup(QString const& name, QString const& label)
{
	currentGroup = name;

	auto newScrollArea = make_qt_unique<QScrollArea>(*this);
	auto newTab        = make_qt_unique<QWidget>(*this);
	currentForm        = make_qt_unique<QFormLayout>(*newTab);
	currentForm->setSizeConstraint(QLayout::SetMinimumSize);

	newScrollArea->setWidget(newTab);

	QTabWidget::addTab(newScrollArea, label);

	orderedGroups.append(name);
}

void SettingsWidget::insertGroup(QString const& name, QString const& label,
                                 int index)
{
	currentGroup = name;

	auto newScrollArea = make_qt_unique<QScrollArea>(*this);
	auto newTab        = make_qt_unique<QWidget>(*this);
	currentForm        = make_qt_unique<QFormLayout>(*newTab);
	currentForm->setSizeConstraint(QLayout::SetMinimumSize);

	newScrollArea->setWidget(newTab);

	QTabWidget::insertTab(index, newScrollArea, label);

	orderedGroups.insert(index, name);
}

void SettingsWidget::editGroup(QString const& name, bool ignoreEngineSeparation)
{
	currentGroup = name;

	unsigned int index(orderedGroups.indexOf(name));

	currentForm = dynamic_cast<QFormLayout*>(
	    dynamic_cast<QScrollArea*>(QTabWidget::widget(index))
	        ->widget()
	        ->layout());

	if(ignoreEngineSeparation)
	{
		return;
	}

	currentForm->insertRow(0, tr("ENGINE"), make_qt_unique<QWidget>(*this));
	currentForm->addRow(" ", make_qt_unique<QWidget>(*this));
	currentForm->addRow(PROJECT_NAME, make_qt_unique<QWidget>(*this));
}

void SettingsWidget::addCustomGroup(QString const& name, QString const& label,
                                    QWidget* w)
{
	currentGroup = name;

	QTabWidget::addTab(w, label);

	orderedGroups.append(name);
}

void SettingsWidget::insertCustomGroup(QString const& name,
                                       QString const& label, int index,
                                       QWidget* w)
{
	currentGroup = name;

	QTabWidget::insertTab(index, w, label);

	orderedGroups.insert(index, name);
}

void SettingsWidget::addBoolSetting(QString const& name, bool defaultVal,
                                    QString const& label)
{
	QString fullName(currentGroup + '/' + name);

	if(!settings.contains(fullName))
	{
		settings.setValue(fullName, defaultVal);
	}

	auto checkBox = make_qt_unique<QCheckBox>(*this);
	checkBox->setCheckState(settings.value(fullName).toBool() ? Qt::Checked
	                                                          : Qt::Unchecked);
	connect(checkBox, &QCheckBox::stateChanged, this,
	        [this, fullName](int s)
	        { updateValue(fullName, s != Qt::Unchecked); });

	currentForm->addRow(label + " :", checkBox);
}

void SettingsWidget::addUIntSetting(QString const& name,
                                    unsigned int defaultVal,
                                    QString const& label, unsigned int minVal,
                                    unsigned int maxVal)
{
	QString fullName(currentGroup + '/' + name);

	if(!settings.contains(fullName))
	{
		settings.setValue(fullName, defaultVal);
	}

	auto sbox = make_qt_unique<QSpinBox>(*this);
	sbox->setRange(minVal, maxVal);
	sbox->setValue(settings.value(fullName).toUInt());

	connect(sbox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
	        this,
	        [this, fullName](unsigned int v) { updateValue(fullName, v); });

	currentForm->addRow(label + " :", sbox);
}

void SettingsWidget::addIntSetting(QString const& name, int defaultVal,
                                   QString const& label, int minVal, int maxVal)
{
	QString fullName(currentGroup + '/' + name);

	if(!settings.contains(fullName))
	{
		settings.setValue(fullName, defaultVal);
	}

	auto sbox = make_qt_unique<QSpinBox>(*this);
	sbox->setRange(minVal, maxVal);
	sbox->setValue(settings.value(fullName).toInt());

	connect(sbox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
	        this, [this, fullName](int v) { updateValue(fullName, v); });

	currentForm->addRow(label + " :", sbox);
}

void SettingsWidget::addDoubleSetting(QString const& name, double defaultVal,
                                      QString const& label, double minVal,
                                      double maxVal, unsigned int decimals)
{
	QString fullName(currentGroup + '/' + name);

	if(!settings.contains(fullName))
	{
		settings.setValue(fullName, defaultVal);
	}

	auto sbox = make_qt_unique<QDoubleSpinBox>(*this);
	sbox->setRange(minVal, maxVal);
	sbox->setDecimals(decimals);
	sbox->setValue(settings.value(fullName).toDouble());

	connect(sbox,
	        static_cast<void (QDoubleSpinBox::*)(double)>(
	            &QDoubleSpinBox::valueChanged),
	        this, [this, fullName](double v) { updateValue(fullName, v); });

	currentForm->addRow(label + " :", sbox);
}

void SettingsWidget::addStringSetting(QString const& name,
                                      QString const& defaultVal,
                                      QString const& label, bool password)
{
	QString fullName(currentGroup + '/' + name);

	if(!settings.contains(fullName))
	{
		settings.setValue(fullName, defaultVal);
	}

	auto lineEdit = make_qt_unique<QLineEdit>(*this);
	lineEdit->setText(settings.value(fullName).toString());
	lineEdit->setMinimumWidth(400);
	if(password)
	{
		lineEdit->setEchoMode(QLineEdit::Password);
	}

	connect(lineEdit, &QLineEdit::textChanged, this,
	        [this, fullName](QString const& t) { updateValue(fullName, t); });

	currentForm->addRow(label + " :", lineEdit);
}

QComboBox* SettingsWidget::addStringAmongListSetting(
    QString const& name, QStringList const& values,
    QStringList const& strLabels, QString const& label,
    unsigned int defaultIndex)
{
	QString fullName(currentGroup + '/' + name);

	QString currentVal(settings.value(fullName).toString());
	int currentIndex(values.indexOf(currentVal));

	if(!settings.contains(fullName) || currentIndex < 0)
	{
		settings.setValue(fullName, values[defaultIndex]);
		currentIndex = defaultIndex;
	}

	auto comboBox = make_qt_unique<QComboBox>(*this);
	for(int i(0); i < strLabels.size(); ++i)
	{
		auto const& label(strLabels[i]);
		auto const& val(values[i]);
		comboBox->addItem(label, val);
	}
	comboBox->setCurrentIndex(currentIndex);

	connect(
	    comboBox,
	    static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
	    this,
	    [this, fullName, comboBox](int index)
	    {
		    auto str = comboBox->itemData(index).toString();
		    updateValue(fullName, str);
	    });

	currentForm->addRow(label + " : ", comboBox);

	return comboBox;
}

void SettingsWidget::addFilePathSetting(QString const& name,
                                        QString const& defaultVal,
                                        QString const& label,
                                        QString const& filter)
{
	QString fullName(currentGroup + '/' + name);

	if(!settings.contains(fullName))
	{
		settings.setValue(fullName, defaultVal);
	}

	auto lineEdit = make_qt_unique<QLineEdit>(*this);
	lineEdit->setText(settings.value(fullName).toString());
	lineEdit->setMinimumWidth(400);

	auto dirModel = make_qt_unique<QFileSystemModel>(*this);
	dirModel->setRootPath(QDir::currentPath());
	auto completer = make_qt_unique<QCompleter>(*this, dirModel);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setCompletionMode(QCompleter::PopupCompletion);
	lineEdit->setCompleter(completer);

	auto browsePb = make_qt_unique<QPushButton>(*this);
	browsePb->setText("...");
	connect(browsePb, &QPushButton::clicked, this,
	        [this, label, lineEdit, filter](bool)
	        {
		        QString result(QFileDialog::getOpenFileName(
		            this, label, lineEdit->text(), filter));
		        if(result != "")
		        {
			        lineEdit->setText(result);
		        }
	        });

	auto w      = make_qt_unique<QWidget>(*this);
	auto layout = make_qt_unique<QHBoxLayout>(*w);
	layout->addWidget(lineEdit);
	layout->addWidget(browsePb);

	connect(lineEdit, &QLineEdit::textChanged, this,
	        [this, fullName](QString const& t) { updateValue(fullName, t); });

	currentForm->addRow(label + " :", w);
}

void SettingsWidget::addDirPathSetting(QString const& name,
                                       QString const& defaultVal,
                                       QString const& label)
{
	QString fullName(currentGroup + '/' + name);

	if(!settings.contains(fullName))
	{
		settings.setValue(fullName, defaultVal);
	}

	auto lineEdit = make_qt_unique<QLineEdit>(*this);
	lineEdit->setText(settings.value(fullName).toString());
	lineEdit->setFixedWidth(350);

	auto dirModel = make_qt_unique<QFileSystemModel>(*this);
	dirModel->setRootPath(QDir::currentPath());
	auto completer = make_qt_unique<QCompleter>(*this, dirModel);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setCompletionMode(QCompleter::PopupCompletion);
	lineEdit->setCompleter(completer);

	auto browsePb = make_qt_unique<QPushButton>(*this);
	browsePb->setText("...");
	connect(browsePb, &QPushButton::clicked, this,
	        [this, label, lineEdit](bool)
	        {
		        QString result(QFileDialog::getExistingDirectory(
		            this, label, lineEdit->text()));
		        if(result != "")
		        {
			        lineEdit->setText(result);
		        }
	        });

	auto w      = make_qt_unique<QWidget>(*this);
	auto layout = make_qt_unique<QHBoxLayout>(*w);
	layout->addWidget(lineEdit);
	layout->addWidget(browsePb);

	connect(lineEdit, &QLineEdit::textChanged, this,
	        [this, fullName](QString const& t) { updateValue(fullName, t); });

	currentForm->addRow(label + " :", w);
}

void SettingsWidget::addVector3DSetting(QString const& name,
                                        QVector3D const& defaultVal,
                                        QString const& label,
                                        std::array<QString, 3> componentLabels,
                                        float minVal, float maxVal)
{
	QString fullName(currentGroup + '/' + name);

	if(!settings.contains(fullName))
	{
		settings.setValue(fullName, defaultVal);
	}

	auto stored(settings.value(fullName).value<QVector3D>());

	auto w                                = make_qt_unique<QWidget>(*this);
	auto layout                           = make_qt_unique<QHBoxLayout>(*w);
	std::array<QDoubleSpinBox*, 3> sboxes = {{nullptr, nullptr, nullptr}};
	unsigned int i(0);
	for(auto& sbox : sboxes)
	{
		sbox = make_qt_unique<QDoubleSpinBox>(*this);
		sbox->setRange(minVal, maxVal);
		sbox->setSingleStep((maxVal - minVal) / 100.f);
		sbox->setValue(stored[i]);
		layout->addWidget(
		    make_qt_unique<QLabel>(*this, componentLabels.at(i) + " :"));
		layout->addWidget(sbox);
		++i;
	}
	for(auto sbox : sboxes)
	{
		connect(sbox,
		        static_cast<void (QDoubleSpinBox::*)(double)>(
		            &QDoubleSpinBox::valueChanged),
		        this,
		        [this, fullName, sboxes](double)
		        {
			        updateValue(fullName, QVector3D(sboxes[0]->value(),
			                                        sboxes[1]->value(),
			                                        sboxes[2]->value()));
		        });
	}

	currentForm->addRow(label + " :", w);
}

void SettingsWidget::addColorSetting(QString const& name,
                                     QColor const& defaultVal,
                                     QString const& label)
{
	QString fullName(currentGroup + '/' + name);

	if(!settings.contains(fullName))
	{
		settings.setValue(fullName, defaultVal);
	}

	auto stored(settings.value(fullName).value<QColor>());

	auto colorSelector = make_qt_unique<ColorSelector>(*this, label);

	colorSelector->setStyleSheet("QPushButton{ \
    background-color: " + stored.name()
	                             + ";        \
    border-style: inset;                     \
    }");

	connect(colorSelector, &ColorSelector::colorChanged, this,
	        [this, fullName](QColor const& color)
	        { updateValue(fullName, color); });

	currentForm->addRow(label + " :", colorSelector);
}

void SettingsWidget::addDateTimeSetting(QString const& name,
                                        QDateTime const& defaultVal,
                                        QString const& label)
{
	QString fullName(currentGroup + '/' + name);

	if(!settings.contains(fullName))
	{
		settings.setValue(fullName, defaultVal);
	}

	QDateTime stored(settings.value(fullName).value<QDateTime>().toTimeSpec(
	    Qt::OffsetFromUTC));

	auto w      = make_qt_unique<QWidget>(*this);
	auto layout = make_qt_unique<QHBoxLayout>(*w);

	auto dtEdit = make_qt_unique<QDateTimeEdit>(*this, stored);
	dtEdit->setCalendarPopup(true);
	dtEdit->setDisplayFormat("dd.MM.yyyy hh:mm:ss");

	connect(dtEdit, &QDateTimeEdit::dateTimeChanged, this,
	        [this, fullName](QDateTime dt)
	        { updateValue(fullName, std::move(dt)); });

	auto now = make_qt_unique<QPushButton>(*this, tr("Now"));
	connect(now, &QPushButton::clicked, this,
	        [dtEdit]()
	        { dtEdit->setDateTime(QDateTime::currentDateTimeUtc()); });

	layout->addWidget(dtEdit);
	layout->addWidget(now);
	currentForm->addRow(label + " :", w);
}

void SettingsWidget::addScreenSetting(QString const& name,
                                      QString const& defaultVal,
                                      QString const& label)
{
	QString fullName(currentGroup + '/' + name);

	if(!settings.contains(fullName))
	{
		settings.setValue(fullName, defaultVal);
	}

	QString stored(settings.value(fullName).toString());

	auto w      = make_qt_unique<QWidget>(*this);
	auto layout = make_qt_unique<QHBoxLayout>(*w);

	auto l = make_qt_unique<QLabel>(*this);
	l->setText(stored == "" ? "AUTO" : stored);

	auto button = make_qt_unique<QPushButton>(*this);
	button->setText("...");

	connect(button, &QPushButton::clicked, this,
	        [this, l, fullName](bool)
	        {
		        auto screen = ScreenSelector::selectScreen(this);
		        updateValue(fullName, screen);
		        l->setText(screen == "" ? "AUTO" : screen);
	        });

	layout->addWidget(l);
	layout->addWidget(button);
	currentForm->addRow(label + " :", w);
}

void SettingsWidget::addWindowsDefinitionSettings(
    QString const& name, QList<RenderingWindow::Parameters> const& defaultVal,
    QString const& label)
{
	// def useful lambdas
	auto valToVariant = [](QList<RenderingWindow::Parameters> const& val)
	{
		QStringList variantList;
		for(auto const& v : val)
		{
			variantList << v.toStr();
		}
		return variantList;
	};
	auto variantToVal = [](QStringList const& variantList)
	{
		QList<RenderingWindow::Parameters> val;
		for(auto const& var : variantList)
		{
			RenderingWindow::Parameters p;
			p.fromStr(var);
			val << p;
		}
		return val;
	};
	// end of lambdas

	QString fullName(currentGroup + '/' + name);

	if(!settings.contains(fullName))
	{
		settings.setValue(fullName, valToVariant(defaultVal));
	}

	windowsParams = variantToVal(settings.value(fullName).toStringList());
	if(windowsParams.isEmpty())
	{
		windowsParams << RenderingWindow::Parameters{};
	}

	auto tab = make_qt_unique<QTabWidget>(*this);
	tab->setMaximumWidth(400);
	tab->setTabsClosable(windowsParams.size() > 1);
	for(int i(0); i < windowsParams.size(); ++i)
	{
		auto windowParamsSelector = make_qt_unique<WindowParametersSelector>(
		    *this, windowsParams.at(i));
		RenderingWindow::Parameters* p = &windowsParams[i];

		connect(windowParamsSelector,
		        &WindowParametersSelector::parametersChanged,
		        [this, fullName, p,
		         &valToVariant](RenderingWindow::Parameters const& params)
		        {
			        *p = params;
			        updateValue(fullName, valToVariant(windowsParams));
		        });

		if(i == 0)
		{
			tab->addTab(windowParamsSelector, tr("Main"));
		}
		else
		{
			tab->addTab(windowParamsSelector,
			            tr("Subwindow ") + QString::number(i));
		}
	}

	connect(tab, &QTabWidget::tabCloseRequested,
	        [this, tab, fullName, &valToVariant](int index)
	        {
		        tab->setCurrentIndex(index);
		        if(QMessageBox::question(
		               this, tr("Remove Window"),
		               tr("Are you sure about removing this window ?"))
		           != QMessageBox::StandardButton::Yes)
		        {
			        return;
		        }

		        tab->removeTab(index);
		        windowsParams.removeAt(index);
		        tab->setTabsClosable(windowsParams.size() > 1);
		        updateValue(fullName, valToVariant(windowsParams));

		        for(int i(0); i < tab->count(); ++i)
		        {
			        if(i == 0)
			        {
				        tab->setTabText(i, tr("Main"));
			        }
			        else
			        {
				        tab->setTabText(i,
				                        tr("Subwindow ") + QString::number(i));
			        }
		        }
	        });

	auto button = make_qt_unique<QPushButton>(*this);
	button->setText("+");
	connect(button, &QPushButton::pressed,
	        [this, fullName, &valToVariant, tab]()
	        {
		        auto windowParamsSelector
		            = make_qt_unique<WindowParametersSelector>(*this);
		        windowsParams.append(RenderingWindow::Parameters{});
		        tab->setTabsClosable(windowsParams.size() > 1);
		        updateValue(fullName, valToVariant(windowsParams));
		        RenderingWindow::Parameters* p = &windowsParams.last();

		        connect(windowParamsSelector,
		                &WindowParametersSelector::parametersChanged,
		                [this, fullName, p, &valToVariant](
		                    RenderingWindow::Parameters const& params)
		                {
			                *p = params;
			                updateValue(fullName, valToVariant(windowsParams));
		                });

		        tab->addTab(windowParamsSelector,
		                    tr("Subwindow ")
		                        + QString::number(windowsParams.size() - 1));
	        });
	tab->setCornerWidget(button);

	currentForm->addRow(label + " :", tab);
}

void SettingsWidget::addKeySequenceSetting(QString const& name,
                                           QKeySequence const& defaultVal,
                                           QString const& label)
{
	QString fullName(currentGroup + '/' + name);

	if(!settings.contains(fullName))
	{
		settings.setValue(fullName,
		                  defaultVal.toString(QKeySequence::PortableText));
	}

	auto keyseqEdit = make_qt_unique<QKeySequenceEdit>(
	    *this, QKeySequence(settings.value(fullName).toString(),
	                        QKeySequence::PortableText));
	keyseqEdit->setMinimumWidth(100);

	connect(keyseqEdit, &QKeySequenceEdit::keySequenceChanged, this,
	        [this, fullName](QKeySequence const& t)
	        { updateValue(fullName, t); });

	currentForm->addRow(label + " :", keyseqEdit);
}

void SettingsWidget::addLanguageSetting(QString const& name,
                                        QString const& defaultVal,
                                        QString const& label)
{
	QString fullName(currentGroup + '/' + name);
	if(!settings.contains(fullName))
	{
		settings.setValue(fullName, defaultVal);
	}

	auto stored = settings.value(fullName).toString();

	QList<QPair<QString, QString>> available(
	    {{"en", QLocale("en").nativeLanguageName()}});
	for(auto const& file :
	    QDir("data/translations").entryList({"HydrogenVR_*.qm"}, QDir::Files))
	{
		QString name(file.split('_')[1].split(".")[0]);
		QPair<QString, QString> elem(
		    {name, QLocale(name).nativeLanguageName()});
		elem.second[0] = elem.second[0].toUpper();
		available.push_back(elem);
	}

	auto comboBox = make_qt_unique<QComboBox>(*this);
	for(auto const& pair : available)
	{
		comboBox->addItem(pair.second, pair.first);

		if(QLocale(stored).language() == QLocale(pair.first).language())
		{
			comboBox->setCurrentText(pair.second);
		}
	}

	void (QComboBox::*indexChangedSignal)(int)
	    = &QComboBox::currentIndexChanged;
	connect(comboBox, indexChangedSignal, this,
	        [this, fullName, available](int index)
	        { updateValue(fullName, available[index].first); });

	currentForm->addRow(label + " :", comboBox);
}

void SettingsWidget::showEvent(QShowEvent* /*event*/)
{
	maxWidgetSize.setWidth(0);
	maxWidgetSize.setHeight(0);
	for(int i(0); i < orderedGroups.size(); ++i)
	{
		setCurrentIndex(i);
		auto w = dynamic_cast<QScrollArea*>(QTabWidget::widget(i))->widget();
		maxWidgetSize.setWidth(fmaxf(maxWidgetSize.width(), w->size().width()));
		maxWidgetSize.setHeight(
		    fmaxf(maxWidgetSize.height(), w->size().height()));
	}
	setCurrentIndex(0);

	emit maxWidgetSizeChanged(maxWidgetSize);
}
