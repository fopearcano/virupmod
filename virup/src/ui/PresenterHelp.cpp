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

#include "ui/PresenterHelp.hpp"

#include <QPushButton>
#include <QVBoxLayout>

#include "MainWin.hpp"

PresenterHelp::PresenterHelp(MainWin& mainwin)
    : VIRUPDialog3D({0.7f, 0.5f})
    , mainwin(mainwin)
{
	setWindowTitle(tr("Presenter Help"));
	setFixedSize(300, 100);

	auto mainLayout = new QVBoxLayout(this);

	auto b = new QPushButton(this);
	b->setText(tr("Refresh Fullscreen"));
	connect(b, &QPushButton::pressed,
	        [this]() { this->mainwin.refreshFullscreen(); });
	mainLayout->addWidget(b);

	b = new QPushButton(this);
	b->setText(tr("Quit"));
	connect(b, &QPushButton::pressed, [this]() { this->mainwin.close(); });
	mainLayout->addWidget(b);
}
