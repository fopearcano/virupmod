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

#ifndef DIALOG3D_HPP
#define DIALOG3D_HPP

#include <QDialog>
#include <QTest>

#include "Widget3D.hpp"
#include "vr/Controller.hpp"

class Dialog3D : public QDialog
{
  public:
	Dialog3D();
	void installEventFilters();
	void showFromHeadset(VRHandler const& vrHandler);
	void showFromController(Controller const& controller);
	void triggerPressed(Controller const& controller);
	void triggerReleased(Controller const& controller);
	void click(Controller const& controller);
	void render(VRHandler const& vrHandler, ToneMappingModel const& tmm);
	virtual ~Dialog3D() = default;

  protected:
	virtual void paintEvent(QPaintEvent* event) override;
	void mouseMove(QPointF const& relativePosition);
	void mousePress(QPointF const& relativePosition);
	void mouseRelease(QPointF const& relativePosition);
	void mouseClick(QPointF const& relativePosition);
	bool eventFilter(QObject* obj, QEvent* event) override;
	void installEventFilters(QObject* obj);

  private:
	// returned z() := distance from position of controller to dialog
	QVector3D intersection(Controller const& controller) const;

	Widget3D widget3d;
	GLMesh pointer;
	GLShaderProgram shader;

	Side sidePriority = Side::LEFT;

	const float sqrt2over2 = sqrt(2.f) / 2.f;
};

#endif // DIALOG3D_HPP
