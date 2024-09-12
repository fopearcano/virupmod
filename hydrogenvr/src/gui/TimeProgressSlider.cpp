/*
    Copyright (C) 2023 Florian Cabot <florian.cabot@hotmail.fr>

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

#include "gui/TimeProgressSlider.hpp"

#include <QMouseEvent>
#include <QStyle>

TimeProgressSlider::TimeProgressSlider(Qt::Orientation orientation,
                                       QWidget* parent)
    : QSlider(orientation, parent)
{
	connect(this, &QSlider::sliderPressed,
	        [this]() { ignoreTimeUpdate = true; });
	connect(this, &QSlider::sliderReleased,
	        [this]() { ignoreTimeUpdate = false; });
	connect(this, &QSlider::sliderMoved,
	        [this](int value) { emit userPickedTime(value); });
}

void TimeProgressSlider::updateTime(int time)
{
	if(ignoreTimeUpdate)
	{
		return;
	}
	blockSignals(true);
	setValue(time);
	blockSignals(false);
}

void TimeProgressSlider::mouseMoveEvent(QMouseEvent* e)
{
	const int value(
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	    QStyle::sliderValueFromPosition(minimum(), maximum(), e->position().x(),
	                                    width()));
#else
	    QStyle::sliderValueFromPosition(minimum(), maximum(), e->x(), width()));
#endif
	emit userHoversTime(value);
	QSlider::mouseMoveEvent(e);
}

void TimeProgressSlider::mousePressEvent(QMouseEvent* e)
{
	QSlider::mousePressEvent(e);
	if(ignoreTimeUpdate)
	{
		return;
	}
	const int value(
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	    QStyle::sliderValueFromPosition(minimum(), maximum(), e->position().x(),
	                                    width()));
#else
	    QStyle::sliderValueFromPosition(minimum(), maximum(), e->x(), width()));
#endif
	emit userPickedTime(value);
}
