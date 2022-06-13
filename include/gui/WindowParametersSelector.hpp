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

#ifndef WINDOWPARAMETERSSELECTOR_HPP
#define WINDOWPARAMETERSSELECTOR_HPP

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>

#include "RenderingWindow.hpp"
#include "gui/ScreenSelector.hpp"

class WindowParametersSelector : public QWidget
{
	Q_OBJECT
  public:
	WindowParametersSelector(QWidget* parent = nullptr,
	                         RenderingWindow::Parameters const& initialValue
	                         = {});

  signals:
	void parametersChanged(RenderingWindow::Parameters params);

  public slots:
	void setParameters(RenderingWindow::Parameters const& params);

  private:
	RenderingWindow::Parameters value;

	QSpinBox* widthSpinBox;
	QSpinBox* heightSpinBox;
	QCheckBox* fullscreenCBox;
	QLabel* screenLabel;
	QDoubleSpinBox* hAngleShiftSpinBox;
	QDoubleSpinBox* vAngleShiftSpinBox;
	QCheckBox* forceLeftCBox;
	QCheckBox* forceRightCBox;
};

#endif // WINDOWPARAMETERSSELECTOR_HPP
