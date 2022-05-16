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

#include "ui/PlanetarySystemSelector.hpp"

PlanetarySystemSelector::PlanetarySystemSelector(Universe const& universe,
                                                 Animator& animator)
    : universe(universe)
    , animator(animator)
    , tree(this)
{
	setFixedSize(250, 600);
	setWindowTitle(tr("Orbital Systems List"));

	auto layout = new QVBoxLayout(this);

	auto w            = new QWidget(this);
	auto layoutSearch = new QHBoxLayout(w);

	auto searchLabel = new QLabel(w);
	searchLabel->setText(tr("Search :"));
	auto searchBar = new QLineEdit(w);
	connect(searchBar, &QLineEdit::textChanged, this,
	        &PlanetarySystemSelector::setVisibleItems);
	layoutSearch->addWidget(searchLabel);
	layoutSearch->addWidget(searchBar);
	layout->addWidget(w);

	tree.setColumnCount(1);
	tree.setHeaderLabel(QString());
	connect(&tree, &QTreeWidget::itemActivated, this,
	        &PlanetarySystemSelector::selectOrbitable);
	layout->addWidget(&tree);

	for(auto const& sysName : universe.planetSystems->getSystemsNames())
	{
		auto item = new QTreeWidgetItem(&tree);
		item->setText(0, sysName);
		item->addChild(constructItems(
		    universe.planetSystems->getSystem(sysName)->getRootOrbitable(),
		    item));
		topLevelItems.push_back(item);
	}
	auto b = new QPushButton(this);
	b->setText(tr("Go !"));
	connect(b, &QPushButton::pressed,
	        [this]() { selectOrbitable(tree.currentItem(), 0); });
	layout->addWidget(b);
	installEventFilters();
}

void PlanetarySystemSelector::setVisibleItems(QString const& match)
{
	for(auto item : topLevelItems)
	{
		if(match == ""
		   || item->text(0).replace('-', ' ').contains(
		          QString(match).replace('-', ' '), Qt::CaseInsensitive))
		{
			item->setHidden(false);
		}
		else
		{
			item->setHidden(true);
		}
	}
}

QTreeWidgetItem*
    PlanetarySystemSelector::constructItems(Orbitable const& orbitable,
                                            QTreeWidgetItem* parent)
{
	QTreeWidgetItem* item;
	if(parent == nullptr)
	{
		item = new QTreeWidgetItem(&tree);
	}
	else
	{
		item = new QTreeWidgetItem(parent);
	}

	if(orbitable.getOrbitableType() == Orbitable::Type::SPACECRAFT)
	{
		auto obt(orbitable.getOrbit());
		if(obt != nullptr)
		{
			QString beginDate(
			    SimulationTime::utToDateTime(obt->getRange().first)
			        .date()
			        .toString(Qt::ISODate));
			QString endDate(SimulationTime::utToDateTime(obt->getRange().second)
			                    .date()
			                    .toString(Qt::ISODate));
			if(obt->isInRange(universe.getClock().getCurrentUt()))
			{
				item->setText(0, QString::fromStdString(orbitable.getName())
				                     + "\n(" + beginDate + "\n->" + endDate
				                     + ")");
				item->setForeground(0, QColor("green"));
			}
			else
			{
				item->setText(0, QString::fromStdString(orbitable.getName())
				                     + "\n(" + beginDate + "\n->" + endDate
				                     + ")");
				item->setForeground(0, QColor("red"));
			}
		}
		else
		{
			item->setText(0, QString::fromStdString(orbitable.getName()));
			std::cout << std::string("Spacecraft \"") + orbitable.getName()
			                 + "\" doesn't have an orbit"
			          << std::endl;
		}
	}
	else if(orbitable.getOrbitableType() == Orbitable::Type::BINARY)
	{
		delete item;
		for(auto child : orbitable.getChildren())
		{
			parent->addChild(constructItems(*child, parent));
		}
		return parent;
	}
	else
	{
		item->setText(0, QString::fromStdString(orbitable.getName()));
	}

	for(auto child : orbitable.getChildren())
	{
		item->addChild(constructItems(*child, item));
	}

	return item;
}

void PlanetarySystemSelector::selectOrbitable(QTreeWidgetItem* item, int column)
{
	auto rootItem = item;
	while(rootItem->parent() != nullptr)
	{
		rootItem = rootItem->parent();
	}

	if(rootItem == item)
	{
		item = rootItem->child(0);
	}

	QString name(item->text(column));
	if(name.contains('('))
	{
		unsigned int pos(name.lastIndexOf('('));
		name = name.left(pos - 1);
	}

	auto body = (*universe.planetSystems->getSystem(
	    rootItem->text(column)))[name.toStdString()];

	if(body->getOrbit() != nullptr
	   && !body->getOrbit()->isInRange(universe.getClock().getCurrentUt()))
	{
		if(body->getOrbitableType() == Orbitable::Type::SPACECRAFT)
		{
			item->setForeground(0, QColor("red"));
		}
		return;
	}
	if(body->getOrbitableType() == Orbitable::Type::SPACECRAFT)
	{
		item->setForeground(0, QColor("green"));
	}

	auto radius = dynamic_cast<CelestialBody const*>(body)
	                  ->getCelestialBodyParameters()
	                  .radius;

	auto scene = animator.getCurrentScene();
	SceneSpatialData sd(universe, rootItem->text(column), name, 3.0f * radius);
	animator.executeTransition(Transition(
	    Scene(sd, SceneTemporalData(scene.getTemporalData().getTimeCoeff()),
	          scene.getUI()),
	    10.f));
}
