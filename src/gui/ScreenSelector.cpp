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

#include "gui/ScreenSelector.hpp"

QString& ScreenSelector::retValue()
{
	static QString retValue = "";
	return retValue;
}

QString ScreenSelector::selectScreen(QWidget* parent)
{
	retValue() = "";
	ScreenSelector select(parent);
	select.exec();
	return retValue();
}

ScreenSelector::ScreenSelector(QWidget* parent)
    : QDialog(parent)
{
	float aspectRatio(static_cast<float>(desktopGeometry.width())
	                  / desktopGeometry.height());
	h = static_cast<int>(w / aspectRatio);

	this->setFixedSize(QSize(w, h));
	for(auto const& s : getScreens())
	{
		auto button = new QPushButton(this);
		button->setGeometry(s.second);
		button->setText(s.first);

		connect(button, &QPushButton::clicked, this, [this, s](bool) {
			retValue() = s.first;
			this->close();
		});
	}
}

QList<QPair<QString, QRect>> ScreenSelector::getScreens() const
{
	QList<QPair<QString, QRect>> result;
	QList<QScreen*> screens(QGuiApplication::screens());
	for(auto s : screens)
	{
		QRect geom(s->geometry());
		geom.setX((s->geometry().x() - desktopGeometry.x()) * w
		          / desktopGeometry.width());
		geom.setWidth(s->geometry().width() * w / desktopGeometry.width());
		geom.setY((s->geometry().y() - desktopGeometry.y()) * h
		          / desktopGeometry.height());
		geom.setHeight(s->geometry().height() * h / desktopGeometry.height());

		result.append({s->name(), geom});
	}
	return result;
}
