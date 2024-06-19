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

#include <QFormLayout>

UniverseElementSelector::UniverseElementSelector(Universe& universe,
                                                 Animator& animator)
    : VIRUPDialog3D({0.65f, 0.f})
    , universe(universe)
    , animator(animator)
    , listWidget(this)
{
	setFixedSize(250, 600);
	setWindowTitle(tr("Cosmological Elements List"));

	auto layout = make_qt_unique<QVBoxLayout>(*this);

	connect(&listWidget, &QListWidget::itemActivated, this,
	        &UniverseElementSelector::selectElement);
	layout->addWidget(&listWidget);

	for(auto const& elementName : universe.getUniverseElementsNames())
	{
		listWidget.addItem(elementName);
	}
	auto b = make_qt_unique<QPushButton>(*this);
	b->setText(tr("Go !"));
	connect(b, &QPushButton::pressed,
	        [this]() { selectElement(listWidget.currentItem()); });
	layout->addWidget(b);

	b = make_qt_unique<QPushButton>(*this);
	b->setText(tr("Edit"));
	connect(b, &QPushButton::pressed,
	        [this]() { editElement(listWidget.currentItem()); });
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

class UniverseElementEditor : public QDialog
{
  public:
	explicit UniverseElementEditor(UniverseElement* universeElement,
	                               Qt::WindowFlags f = Qt::WindowFlags(),
	                               QWidget* parent   = nullptr)
	    : QDialog(parent, f)
	    , json(universeElement->getJson())
	{
		auto form = make_qt_unique<QFormLayout>(*this);
		for(auto const& pair : UniverseElement::getLauncherFields(*this, json))
		{
			form->addRow(pair.first, pair.second);
		}
		auto type = json["type"].toString();

		QList<QPair<QString, QWidget*>> fields;
		if(type == "cosmolabels")
		{
			fields = CosmologicalLabels::getLauncherFields(*this, json);
		}
		if(type == "csvstars")
		{
			fields = CSVObjects::getStarsLauncherFields(*this, json);
		}
		if(type == "csvgalaxies")
		{
			fields = CSVObjects::getGalaxiesLauncherFields(*this, json);
		}
		if(type == "cosmosim")
		{
			fields = CosmologicalSimulation::getLauncherFields(*this, json);
		}
		if(type == "texsphere")
		{
			fields = TexturedSphere::getLauncherFields(*this, json);
		}
		if(type == "credits")
		{
			fields = Credits::getLauncherFields(*this, json);
		}
		for(auto const& pair : fields)
		{
			form->addRow(pair.first, pair.second);
		}

		auto b = make_qt_unique<QPushButton>(*this);
		b->setText(tr("Apply"));
		form->addRow(b);
		connect(b, &QPushButton::pressed, this,
		        [this, universeElement]()
		        { universeElement->setJson(this->json); });
	};

  private:
	QJsonObject json;
};

void UniverseElementSelector::editElement(QListWidgetItem* item)
{
	UniverseElement* elem(universe.getElement(item->text()));
	auto editor
	    = make_qt_unique<UniverseElementEditor>(*this, elem, Qt::WindowFlags{});
	editor->show();
}
