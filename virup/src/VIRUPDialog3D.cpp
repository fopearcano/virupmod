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

#include "VIRUPDialog3D.hpp"

#include <QScreen>
#include <QWindow>

VIRUPDialog3D::VIRUPDialog3D(QPointF const& screenRelPos)
    : screenRelPos(screenRelPos)
{
}

void VIRUPDialog3D::showEvent(QShowEvent* /*event*/)
{
	auto screenName = QSettings().value("misc/presenterscreen").toString();
	if(screenName.isEmpty())
	{
		return;
	}

	QRect screenGeometry(window()->windowHandle()->screen()->geometry());
	for(auto const* s : QGuiApplication::screens())
	{
		if(s->name() == screenName)
		{
			screenGeometry = s->geometry();
			break;
		}
	}
	auto flags = window()->windowHandle()->flags() & ~Qt::WindowCloseButtonHint;
	window()->windowHandle()->setFlags(flags);
	QRect ownGeometry(geometry());
	ownGeometry.setX(screenGeometry.x()
	                 + screenRelPos.x() * screenGeometry.width());
	ownGeometry.setY(screenGeometry.y()
	                 + screenRelPos.y() * screenGeometry.height());
	setGeometry(ownGeometry);
}
