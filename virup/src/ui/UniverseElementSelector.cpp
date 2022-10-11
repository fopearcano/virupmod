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

#include "ui/UniverseElementSelector.hpp"

UniverseElementSelector::UniverseElementSelector(Universe const& universe,
                                                 Animator& animator)
    : VIRUPDialog3D({0.65f, 0.f})
    , universe(universe)
    , animator(animator)
    , listWidget(this)
{
	setFixedSize(250, 600);
	setWindowTitle(tr("Cosmological Elements List"));

	auto layout = new QVBoxLayout(this);

	connect(&listWidget, &QListWidget::itemActivated, this,
	        &UniverseElementSelector::selectElement);
	layout->addWidget(&listWidget);

	for(auto const& elementName : universe.getUniverseElementsNames())
	{
		listWidget.addItem(elementName);
	}
	auto b = new QPushButton(this);
	b->setText(tr("Go !"));
	connect(b, &QPushButton::pressed,
	        [this]() { selectElement(listWidget.currentItem()); });
	layout->addWidget(b);
	installEventFilters();
}

void UniverseElementSelector::selectElement(QListWidgetItem* item)
{
	auto elem = universe.getElement(item->text());
	auto diameter
	    = elem->getBoundingBox().diameter * elem->unit / universe.mtokpc;

	if(diameter == 0.f)
	{
		qWarning() << item->text() + " doesn't have a bounding box.";
		return;
	}

	auto scene = animator.getCurrentScene();
	SceneSpatialData sd(universe, 0.2f * diameter,
	                    elem->getAbsoluteBBoxCenter());
	SceneUI ui(scene.getUI());
	ui.setVisibility(item->text(), 1.f);
	animator.executeTransition(Transition(
	    Scene(sd, SceneTemporalData(scene.getTemporalData().getTimeCoeff()),
	          ui),
	    10.f));
}
