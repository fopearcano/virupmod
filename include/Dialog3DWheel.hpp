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

#ifndef DIALOG3DWHEEL_HPP
#define DIALOG3DWHEEL_HPP

#include <QPushButton>
#include <QVBoxLayout>

#include "Dialog3D.hpp"

class Dialog3DWheel : public Dialog3D
{
  public:
	Dialog3DWheel(VRHandler const& vrHandler, ToneMappingModel const& tmm);
	void addDialog3D(QString const& name, Dialog3D& dialog3d);
	void vrEvent(VRHandler::Event const& e);
	void render();

  private:
	VRHandler const& vrHandler;
	ToneMappingModel const& tmm;
	QVBoxLayout layout;
	std::vector<Dialog3D*> dialog3Ds;
};

#endif // DIALOG3DWHEEL_HPP
