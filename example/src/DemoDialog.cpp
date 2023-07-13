/*
    Copyright (C) 2021 Florian Cabot <florian.cabot@hotmail.fr>

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

#include "DemoDialog.hpp"

DemoDialog::DemoDialog()
{
	setFixedSize(250, 600);
	setWindowTitle(tr("Demo 3D Dialog"));

	auto layout = make_qt_unique<QVBoxLayout>(*this);

	connect(&listWidget, &QListWidget::itemActivated,
	        [](QListWidgetItem* item) { qDebug() << item->text(); });
	layout->addWidget(&listWidget);

	for(auto const& elementName : QStringList({"Foo", "Bar", "Baz"}))
	{
		listWidget.addItem(elementName);
	}

	auto b = make_qt_unique<QPushButton>(*this);
	b->setText(tr("Close"));
	connect(b, &QPushButton::clicked, [this]() { this->hide(); });
	layout->addWidget(b);
	installEventFilters();
}
