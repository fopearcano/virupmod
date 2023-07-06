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
    : VIRUPDialog3D({0.5f, 0.f})
    , universe(universe)
    , animator(animator)
    , solarSystemTree(this)
    , fullTree(this)
{
	setFixedSize(300, 900);
	setWindowTitle(tr("Orbital Systems List"));

	auto layout = make_qt_unique<QVBoxLayout>(*this);

	auto label = make_qt_unique<QLabel>(*this);
	label->setText(tr("Solar System"));
	layout->addWidget(label);

	solarSystemTree.setColumnCount(1);
	solarSystemTree.setHeaderLabel(QString());
	connect(&solarSystemTree, &QTreeWidget::itemActivated, this,
	        &PlanetarySystemSelector::selectOrbitableSolSys);
	layout->addWidget(&solarSystemTree);

	auto const& sun
	    = universe.planetSystems->getSystem("Solar System")->getRootOrbitable();
	auto item = qt_owned<QTreeWidgetItem>(&solarSystemTree);
	item->setText(0, sun.getName().c_str());
	for(auto childOrb : sun.getChildren())
	{
		item->addChild(
		    constructItems(*childOrb, item, solarSystemTree, universe));
	}
	item->setExpanded(true);

	auto b = make_qt_unique<QPushButton>(*this);
	b->setText(tr("Go !"));
	connect(b, &QPushButton::pressed,
	        [this]()
	        { selectOrbitableSolSys(solarSystemTree.currentItem(), 0); });

	layout->addWidget(b);
	label = make_qt_unique<QLabel>(*this);
	label->setText(tr("Exoplanetary Systems"));
	layout->addWidget(label);

	auto w            = make_qt_unique<QWidget>(*this);
	auto layoutSearch = make_qt_unique<QHBoxLayout>(*w);

	auto searchLabel = make_qt_unique<QLabel>(*w);
	searchLabel->setText(tr("Search :"));
	auto searchBar = make_qt_unique<QLineEdit>(*w);
	connect(searchBar, &QLineEdit::textChanged, this,
	        &PlanetarySystemSelector::setVisibleItems);
	layoutSearch->addWidget(searchLabel);
	layoutSearch->addWidget(searchBar);
	layout->addWidget(w);

	fullTree.setColumnCount(1);
	fullTree.setHeaderLabel(QString());
	connect(&fullTree, &QTreeWidget::itemActivated, this,
	        &PlanetarySystemSelector::selectOrbitableFull);
	layout->addWidget(&fullTree);

	for(auto const& sysName : universe.planetSystems->getSystemsNames())
	{
		if(sysName == "Solar System")
		{
			continue;
		}
		auto item = qt_owned<QTreeWidgetItem>(&fullTree);
		item->setText(0, sysName);
		item->addChild(constructItems(
		    universe.planetSystems->getSystem(sysName)->getRootOrbitable(),
		    item, fullTree, universe));
		topLevelItems.push_back(item);
	}
	b = make_qt_unique<QPushButton>(*this);
	b->setText(tr("Go !"));
	connect(b, &QPushButton::pressed,
	        [this]() { selectOrbitableFull(fullTree.currentItem(), 0); });
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

QTreeWidgetItem* PlanetarySystemSelector::constructItems(
    Orbitable const& orbitable, QTreeWidgetItem* parent, QTreeWidget& tree,
    Universe const& universe)
{
	if(orbitable.getOrbitableType() == Orbitable::Type::BINARY)
	{
		for(auto child : orbitable.getChildren())
		{
			parent->addChild(constructItems(*child, parent, tree, universe));
		}
		return parent;
	}

	QTreeWidgetItem* item;
	if(parent == nullptr)
	{
		item = qt_owned<QTreeWidgetItem>(&tree);
	}
	else
	{
		item = qt_owned<QTreeWidgetItem>(parent);
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
	else
	{
		item->setText(0, QString::fromStdString(orbitable.getName()));
	}

	for(auto child : orbitable.getChildren())
	{
		item->addChild(constructItems(*child, item, tree, universe));
	}

	return item;
}

void PlanetarySystemSelector::selectOrbitable(QTreeWidgetItem* item, int column,
                                              bool solarSystem)
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

	Orbitable const* body;

	if(solarSystem)
	{
		body = (*universe.planetSystems->getSystem(
		    "Solar System"))[name.toStdString()];
	}
	else
	{
		body = (*universe.planetSystems->getSystem(
		    rootItem->text(column)))[name.toStdString()];
	}

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
	if(solarSystem)
	{
		SceneSpatialData sd(universe, "Solar System", name, 3.0f * radius);
		animator.executeTransition(Transition(
		    Scene(sd, SceneTemporalData(scene.getTemporalData().getTimeCoeff()),
		          scene.getUI()),
		    10.f));
	}
	else
	{
		SceneSpatialData sd(universe, rootItem->text(column), name,
		                    3.0f * radius);
		animator.executeTransition(Transition(
		    Scene(sd, SceneTemporalData(scene.getTemporalData().getTimeCoeff()),
		          scene.getUI()),
		    10.f));
	}
}
