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

#include "ui/TimeController.hpp"

#include <QCalendarWidget>

TimeController::TimeController(Universe& universe)
    : universe(universe)
{
	setWindowTitle(tr("Time Controller"));

	auto mainLayout = new QVBoxLayout(this);

	auto w = new QWidget(this);
	mainLayout->addWidget(w);
	auto dateTimeLayout = new QHBoxLayout(w);
	dtEdit              = new QDateTimeEdit(this);
	dtEdit->setCalendarPopup(true);
	dtEdit->setDisplayFormat("dd.MM.yyyy hh:mm:ss");
	dtEdit->setDateTime(universe.getSimulationTime());
	connect(dtEdit, &QDateTimeEdit::dateTimeChanged,
	        [this](QDateTime const& dt)
	        {
		        if(!this->ignoreDTEditUpdate)
		        {
			        this->universe.setSimulationTime(dt);
		        }
	        });
	dateTimeLayout->addWidget(dtEdit);

	timeLabel = new QLabel(this);
	dateTimeLayout->addWidget(timeLabel);

	w = new QWidget(this);
	mainLayout->addWidget(w);
	auto buttonsLayout = new QHBoxLayout(w);

	auto button = new QPushButton(this);
	button->setText(tr("Flip time flow"));
	connect(button, &QPushButton::pressed,
	        [this]() { setTimeCoeffFromUI(-1.f * this->getTimeCoeffForUI()); });
	buttonsLayout->addWidget(button);

	button = new QPushButton(this);
	button->setText(tr("⏪︎"));
	connect(button, &QPushButton::pressed,
	        [this]() { setTimeCoeffFromUI(this->getTimeCoeffForUI() / 10.f); });
	buttonsLayout->addWidget(button);

	button = new QPushButton(this);
	if(timeCoeffBackup == 0.f)
	{
		button->setText(tr("⏸︎"));
	}
	else
	{
		button->setText(tr("⏵︎"));
	}
	connect(button, &QPushButton::pressed,
	        [this, button]()
	        {
		        if(this->universe.getLockedRealTime())
		        {
			        return;
		        }

		        if(timeCoeffBackup == 0.f)
		        {
			        timeCoeffBackup = this->universe.getTimeCoeff();
			        this->universe.setTimeCoeff(0.f);
			        button->setText(tr("⏵︎"));
		        }
		        else
		        {
			        this->universe.setTimeCoeff(timeCoeffBackup);
			        timeCoeffBackup = 0.f;
			        button->setText(tr("⏸︎"));
		        }
	        });
	buttonsLayout->addWidget(button);

	button = new QPushButton(this);
	button->setText(tr("⏩︎"));
	connect(button, &QPushButton::pressed,
	        [this]() { setTimeCoeffFromUI(this->getTimeCoeffForUI() * 10.f); });
	buttonsLayout->addWidget(button);

	button = new QPushButton(this);
	button->setText(tr("x1"));
	connect(button, &QPushButton::pressed,
	        [this]() { setTimeCoeffFromUI(1.f); });
	buttonsLayout->addWidget(button);

	button = new QPushButton(this);
	button->setText(tr("Now"));
	connect(
	    button, &QPushButton::pressed,
	    [this]()
	    { this->universe.setSimulationTime(QDateTime::currentDateTimeUtc()); });
	buttonsLayout->addWidget(button);
}

float TimeController::getTimeCoeffForUI() const
{
	if(timeCoeffBackup == 0)
	{
		return universe.getTimeCoeff();
	}
	return timeCoeffBackup;
}

void TimeController::update()
{
	if(isVisible() && !fixedSize)
	{
		setFixedSize(size());
		fixedSize = true;
	}
	ignoreDTEditUpdate = true;
	if(!dtEdit->calendarWidget()->isVisible())
	{
		dtEdit->setDateTime(universe.getSimulationTime());
	}
	ignoreDTEditUpdate = false;
	QString text;
	text += tr(" x");
	if(timeCoeffBackup == 0.f)
	{
		text += QString::number(universe.getTimeCoeff());
	}
	else
	{
		text += QString::number(timeCoeffBackup) + tr(" (PAUSED)");
	}
	timeLabel->setText(text);
}

void TimeController::setTimeCoeffFromUI(float tc)
{
	if(universe.getLockedRealTime() || fabs(tc) < 1.f || fabs(tc) > 10000000.f)
	{
		return;
	}

	if(timeCoeffBackup == 0.f)
	{
		universe.setTimeCoeff(tc);
	}
	else
	{
		timeCoeffBackup = tc;
	}
}
