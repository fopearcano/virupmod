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

#ifndef VIRUPDIALOG3D_HPP
#define VIRUPDIALOG3D_HPP

#include "Dialog3D.hpp"

class VIRUPDialog3D : public Dialog3D
{
  public:
	VIRUPDialog3D(QPointF const& screenRelPos);

  protected:
	virtual void showEvent(QShowEvent* event) override;

  private:
	QPointF screenRelPos;
};

#endif // VIRUPDIALOG3D_HPP
