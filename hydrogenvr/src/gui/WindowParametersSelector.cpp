/*
    Copyright (C) 2022 Florian Cabot <florian.cabot@hotmail.fr>

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

#include "gui/WindowParametersSelector.hpp"

#include "memory.hpp"

WindowParametersSelector::WindowParametersSelector(QWidget* parent)
    : WindowParametersSelector({}, parent)
{
}

WindowParametersSelector::WindowParametersSelector(
    RenderingWindow::Parameters const& initialValue, QWidget* parent)
    : QWidget(parent)
    , value(initialValue)
    , widthSpinBox(this)
    , heightSpinBox(this)
    , fullscreenCBox(this)
    , screenLabel(this)
    , hAngleShiftSpinBox(this)
    , vAngleShiftSpinBox(this)
    , forceLeftCBox(this)
    , forceRightCBox(this)
{
	auto* mainLayout = make_qt_unique<QFormLayout>(*this);

	widthSpinBox.setRange(0, 17000);
	widthSpinBox.setValue(initialValue.width);
	connect(&widthSpinBox,
	        static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
	        [this](unsigned int v)
	        {
		        value.width = v;
		        emit parametersChanged(value);
	        });
	mainLayout->addRow(tr("Width :"), &widthSpinBox);

	heightSpinBox.setRange(0, 17000);
	heightSpinBox.setValue(initialValue.height);
	connect(&heightSpinBox,
	        static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
	        [this](unsigned int v)
	        {
		        value.height = v;
		        emit parametersChanged(value);
	        });
	mainLayout->addRow(tr("Height :"), &heightSpinBox);

	fullscreenCBox.setChecked(initialValue.fullscreen);
	connect(&fullscreenCBox, &QCheckBox::stateChanged,
	        [this](int s)
	        {
		        value.fullscreen = (s != Qt::Unchecked);
		        emit parametersChanged(value);
	        });
	mainLayout->addRow(tr("Fullscreen :"), &fullscreenCBox);

	auto* w      = make_qt_unique<QWidget>(*this);
	auto* layout = make_qt_unique<QHBoxLayout>(*w);

	screenLabel.setText(
	    initialValue.screenname == "" ? "AUTO" : initialValue.screenname);

	auto* button = make_qt_unique<QPushButton>(*this);
	button->setText("...");

	connect(button, &QPushButton::clicked, this,
	        [this](bool)
	        {
		        const QString screen = ScreenSelector::selectScreen(this);
		        value.screenname     = screen;
		        emit parametersChanged(value);
		        screenLabel.setText(screen == "" ? "AUTO" : screen);
	        });

	layout->setAlignment(Qt::AlignLeft);
	layout->addWidget(&screenLabel);
	layout->addWidget(button);
	mainLayout->addRow(tr("Screen :"), w);

	hAngleShiftSpinBox.setRange(-180.0, 180.0);
	hAngleShiftSpinBox.setDecimals(3);
	hAngleShiftSpinBox.setValue(initialValue.horizontalAngleShift);
	connect(&hAngleShiftSpinBox,
	        static_cast<void (QDoubleSpinBox::*)(double)>(
	            &QDoubleSpinBox::valueChanged),
	        this,
	        [this](double v)
	        {
		        value.horizontalAngleShift = v;
		        emit parametersChanged(value);
	        });
	mainLayout->addRow(tr("Horizontal Shift Angle :"), &hAngleShiftSpinBox);

	vAngleShiftSpinBox.setRange(-180.0, 180.0);
	vAngleShiftSpinBox.setDecimals(3);
	vAngleShiftSpinBox.setValue(initialValue.verticalAngleShift);
	connect(&vAngleShiftSpinBox,
	        static_cast<void (QDoubleSpinBox::*)(double)>(
	            &QDoubleSpinBox::valueChanged),
	        this,
	        [this](double v)
	        {
		        value.verticalAngleShift = v;
		        emit parametersChanged(value);
	        });
	mainLayout->addRow(tr("Vertical Shift Angle :"), &vAngleShiftSpinBox);

	forceLeftCBox.setChecked(initialValue.forceleft);
	connect(&forceLeftCBox, &QCheckBox::stateChanged,
	        [this](int s)
	        {
		        value.forceleft = (s != Qt::Unchecked);
		        emit parametersChanged(value);
	        });
	mainLayout->addRow(tr("Force left eye rendering only :"), &forceLeftCBox);

	forceRightCBox.setChecked(initialValue.forceright);
	connect(&forceRightCBox, &QCheckBox::stateChanged,
	        [this](int s)
	        {
		        value.forceright = (s != Qt::Unchecked);
		        emit parametersChanged(value);
	        });
	mainLayout->addRow(tr("Force right eye rendering only :"), &forceRightCBox);
}

void WindowParametersSelector::setParameters(
    RenderingWindow::Parameters const& params)
{
	value = params;

	widthSpinBox.setValue(value.width);
	heightSpinBox.setValue(value.height);

	emit parametersChanged(value);
}
