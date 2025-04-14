/*
    Copyright (C) 2025 Florian Cabot <florian.cabot@hotmail.fr>

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

#include "ui/ImageCatalogSelector.hpp"

#include "universe/Universe.hpp"

ImageCatalogSelector::ImageCatalogSelector(Universe& universe)
    : VIRUPDialog3D({0.65f, 0.f})
    , universe(universe)
    , listWidget(this)
{
	setFixedSize(250, 600);
	setWindowTitle(tr("Image Catalog List"));

	auto* layout = make_qt_unique<QVBoxLayout>(*this);

	connect(&listWidget, &QListWidget::itemActivated, this,
	        &ImageCatalogSelector::selectElement);
	layout->addWidget(&listWidget);

	for(auto const& elementName : universe.getImages())
	{
		listWidget.addItem(elementName);
	}
	auto* b = make_qt_unique<QPushButton>(*this);
	b->setText(tr("Go !"));
	connect(b, &QPushButton::pressed,
	        [this]() { selectElement(listWidget.currentItem()); });
	layout->addWidget(b);

	installEventFilters();
}

void ImageCatalogSelector::selectElement(QListWidgetItem* item)
{
	universe.setCurrentImage(item->text());
}
