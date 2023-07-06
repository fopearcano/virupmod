/*
    Copyright (C) 2020 Florian Cabot <florian.cabot@hotmail.fr>

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

#include "ui/Visibilities.hpp"

Visibilities::Visibilities(Universe& universe)
    : VIRUPDialog3D({0.8f, 0.f})
    , universe(universe)
{
	setFixedSize(200, 600);
	setWindowTitle(tr("Visibilities List"));

	auto mainLayout = make_qt_unique<QVBoxLayout>(*this);
	auto scrollArea = make_qt_unique<QScrollArea>(*this);

	auto w      = make_qt_unique<QWidget>(*this);
	auto layout = make_qt_unique<QVBoxLayout>(*w);
	layout->setSizeConstraint(QLayout::SetMinimumSize);
	scrollArea->setWidget(w);
	mainLayout->addWidget(scrollArea);

	for(auto const& name : universe.getUniverseElementsNames())
	{
		auto label = make_qt_unique<QLabel>(*this);
		label->setText(name);
		layout->addWidget(label);

		auto slider = make_qt_unique<QSlider>(*this, Qt::Horizontal);
		slider->setMinimum(0);
		slider->setMaximum(100);
		layout->addWidget(slider);
		connect(universe.getElement(name), &UniverseElement::visibilityChanged,
		        [slider](float visibility)
		        { slider->setValue(static_cast<int>(visibility * 100)); });
		connect(slider, &QSlider::valueChanged,
		        [&universe, name](int val)
		        { universe.setVisibility(name, val / 100.f); });
	}

	QStringList nonElementsVisibilities = {
	    {"Constellations", "Orbits", "PlanetsLabels", "Debris", "Asteroids"}};

	for(auto const& name : nonElementsVisibilities)
	{
		auto label = make_qt_unique<QLabel>(*this);
		label->setText(name);
		layout->addWidget(label);

		auto slider = make_qt_unique<QSlider>(*this, Qt::Horizontal);
		slider->setMinimum(0);
		slider->setMaximum(100);
		layout->addWidget(slider);
		connect(&universe, &Universe::nonElementVisibilityChanged,
		        [slider, name](QString const& n, float visibility)
		        {
			        if(n == name)
			        {
				        slider->setValue(static_cast<int>(visibility * 100));
			        }
		        });
		connect(slider, &QSlider::valueChanged,
		        [&universe, name](int val)
		        { universe.setVisibility(name, val / 100.f); });
	}

	installEventFilters();
}
