/*
    Copyright (C) 2023 Florian Cabot <florian.cabot@hotmail.fr>

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

#ifndef FPSCAMERA_HPP
#define FPSCAMERA_HPP

#include "BaseInputManager.hpp"
#include "GamepadHandler.hpp"
#include "camera/BasicCamera.hpp"

class FPSCamera : public BasicCamera
{
  public:
	FPSCamera(VRHandler const& vrHandler);
	void actionEvent(BaseInputManager::Action const& a, bool pressed);
	void mousePressEvent(QMouseEvent* e, QRect const& winGeometry);
	void mouseReleaseEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e, QRect const& winGeometry);
	void updateLookAt(float frameTiming, GamepadHandler const& gamepadHandler);

  private:
	QVector3D getLookDirection() const;

	QVector3D position = {1.f, 1.f, 1.f};

	QVector3D posVel     = {0.f, 0.f, 0.f};
	QVector3D negVel     = {0.f, 0.f, 0.f};
	QVector3D gamepadVel = {0.f, 0.f, 0.f};
	bool moveView        = false;
	float yaw            = 0.f;
	float pitch          = 0.f;

	QPoint cursorPosBackup;
};

#endif // FPSCAMERA_HPP
