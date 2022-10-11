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

#ifndef TIMECONTROLLER_HPP
#define TIMECONTROLLER_HPP

#include <QDateTimeEdit>

#include "Dialog3D.hpp"
#include "universe/Universe.hpp"

class TimeController : public Dialog3D
{
  public:
	TimeController(Universe& universe);
	void update();

  private:
	float getTimeCoeffForUI() const;
	void setTimeCoeffFromUI(float tc);

	Universe& universe;

	QDateTimeEdit* dtEdit;
	bool ignoreDTEditUpdate = false;
	QLabel* timeLabel;

	float timeCoeffBackup = 0.f;
	bool fixedSize        = false;
};

#endif // TIMECONTROLLER_HPP
