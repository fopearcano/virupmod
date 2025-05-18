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

#include "ui/ToneMappingController.hpp"

#include <QPushButton>
#include <QVBoxLayout>

#include "ToneMappingModel.hpp"
#include "universe/UniverseElement.hpp"

ToneMappingController::ToneMappingController(ToneMappingModel& tmm)
    : VIRUPDialog3D({0.3f, 0.5f})
    , tmm(tmm)
    , autoCb(make_qt_unique<QCheckBox>(*this))
    , bmCb(make_qt_unique<QCheckBox>(*this))
    , prkCb(make_qt_unique<QCheckBox>(*this))
    , exposureLabel(make_qt_unique<QLabel>(*this))
{
	setWindowTitle(tr("Tone Mapping Controller"));

	auto* mainLayout = make_qt_unique<QVBoxLayout>(*this);

	auto* w = make_qt_unique<QWidget>(*this);
	mainLayout->QLayout::addWidget(w);
	auto* hl = make_qt_unique<QHBoxLayout>(*w);
	autoCb->setChecked(tmm.autoexposure());
	connect(autoCb, &QCheckBox::stateChanged,
	        [this]() { this->tmm.setAutoexposure(this->autoCb->isChecked()); });
	hl->QLayout::addWidget(autoCb);
	auto* l = make_qt_unique<QLabel>(*this);
	l->setText(tr("Automatic exposure"));
	hl->addWidget(l);

	w = make_qt_unique<QWidget>(*this);
	mainLayout->QLayout::addWidget(w);
	hl = make_qt_unique<QHBoxLayout>(*w);
	bmCb->setChecked(!UniverseElement::useBrightnessMultiplier());
	connect(bmCb, &QCheckBox::stateChanged,
	        [this]()
	        {
		        UniverseElement::useBrightnessMultiplier()
		            = !this->bmCb->isChecked();
	        });
	hl->QLayout::addWidget(bmCb);
	l = make_qt_unique<QLabel>(*this);
	l->setText(tr("Realistic luminosities"));
	hl->addWidget(l);

	w = make_qt_unique<QWidget>(*this);
	mainLayout->QLayout::addWidget(w);
	hl = make_qt_unique<QHBoxLayout>(*w);
	prkCb->setChecked(tmm.purkinje());
	connect(prkCb, &QCheckBox::stateChanged,
	        [this]() { this->tmm.setPurkinje(this->prkCb->isChecked()); });
	hl->QLayout::addWidget(prkCb);
	l = make_qt_unique<QLabel>(*this);
	l->setText(tr("Purkinje Effect"));
	hl->addWidget(l);

	w = make_qt_unique<QWidget>(*this);
	mainLayout->QLayout::addWidget(w);
	hl = make_qt_unique<QHBoxLayout>(*w);
	exposureLabel->setText(tr("Exposure :"));
	hl->addWidget(exposureLabel);
	auto* b = make_qt_unique<QPushButton>(*this);
	b->setText(tr("-"));
	connect(b, &QPushButton::pressed,
	        [this]() { this->tmm.setExposure(this->tmm.exposure() / 1.5f); });
	hl->addWidget(b);
	b = make_qt_unique<QPushButton>(*this);
	b->setText(tr("+"));
	connect(b, &QPushButton::pressed,
	        [this]() { this->tmm.setExposure(this->tmm.exposure() / 1.5f); });
	hl->addWidget(b);

	b = make_qt_unique<QPushButton>(*this);
	b->setText(tr("Reset"));
	connect(b, &QPushButton::pressed,
	        [this]()
	        {
		        this->tmm.setExposure(0.3f);
		        this->tmm.setDynamicrange(10000.f);
		        this->tmm.setAutoexposure(false);
		        this->tmm.setPurkinje(false);
		        UniverseElement::useBrightnessMultiplier() = true;
	        });
	hl->addWidget(b);
}

void ToneMappingController::update()
{
	if(isVisible() && !fixedSize)
	{
		setFixedSize(size());
		fixedSize = true;
	}
	autoCb->setChecked(tmm.autoexposure());
	bmCb->setChecked(!UniverseElement::useBrightnessMultiplier());
	prkCb->setChecked(tmm.purkinje());

	exposureLabel->setText(tr("Exposure : ") + QString::number(tmm.exposure()));
}
