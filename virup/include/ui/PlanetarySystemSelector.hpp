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

#ifndef PLANETARYSYSTEMSELECTOR_HPP
#define PLANETARYSYSTEMSELECTOR_HPP

#include "Dialog3D.hpp"
#include "graphics/OrbitalSystemCamera.hpp"
#include "scenes/Animator.hpp"
#include "universe/Universe.hpp"
#include <QTreeWidgetItem>

class PlanetarySystemSelector : public Dialog3D
{
	Q_OBJECT
  public:
	PlanetarySystemSelector(Universe const& universe, Animator& animator);

  private:
	QTreeWidgetItem* constructItems(Orbitable const& orbitable,
	                                QTreeWidgetItem* parent);
	void selectOrbitable(QTreeWidgetItem* item, int column);
	void setVisibleItems(QString const& match);

	Universe const& universe;
	Animator& animator;
	QTreeWidget tree;

	std::vector<QTreeWidgetItem*> topLevelItems;
};

#endif // PLANETARYSYSTEMSELECTOR_HPP
