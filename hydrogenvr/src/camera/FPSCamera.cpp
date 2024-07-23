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

#include "camera/FPSCamera.hpp"

#include <QMouseEvent>

FPSCamera::FPSCamera(VRHandler const& vrHandler)
    : BasicCamera(vrHandler)
{
}

void FPSCamera::actionEvent(BaseInputManager::Action const& a, bool pressed)
{
	if(pressed)
	{
		if(a.id == "forward")
		{
			negVel.setZ(-1);
		}
		else if(a.id == "left")
		{
			negVel.setX(-1);
		}
		else if(a.id == "backward")
		{
			posVel.setZ(1);
		}
		else if(a.id == "right")
		{
			posVel.setX(1);
		}
	}

	else
	{
		if(a.id == "forward")
		{
			negVel.setZ(0);
		}
		else if(a.id == "left")
		{
			negVel.setX(0);
		}
		else if(a.id == "backward")
		{
			posVel.setZ(0);
		}
		else if(a.id == "right")
		{
			posVel.setX(0);
		}
	}
}

void FPSCamera::mousePressEvent(QMouseEvent* e, QRect const& winGeometry)
{
	if(e->button() == Qt::MouseButton::LeftButton)
	{
		moveView = true;
		// QCursor c(cursor());
		// c.setShape(Qt::CursorShape::BlankCursor);
		cursorPosBackup = QCursor::pos();
		QCursor::setPos(winGeometry.x() + winGeometry.width() / 2,
		                winGeometry.y() + winGeometry.height() / 2);
		// setCursor(c);
	}
}

void FPSCamera::mouseReleaseEvent(QMouseEvent* e)
{
	if(e->button() == Qt::MouseButton::LeftButton)
	{
		moveView = false;
		// QCursor c(cursor());
		// c.setShape(Qt::CursorShape::ArrowCursor);
		QCursor::setPos(cursorPosBackup);
		// setCursor(c);
	}
}

void FPSCamera::mouseMoveEvent(QMouseEvent* e, QRect const& winGeometry)
{
	if(!moveView)
	{
		return;
	}

	float dx = (winGeometry.x() + static_cast<float>(winGeometry.width()) / 2
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	            - e->globalPosition().x())
#else
	            - e->globalX())
#endif
	           / winGeometry.width();
	float dy = (winGeometry.y() + static_cast<float>(winGeometry.height()) / 2
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	            - e->globalPosition().y())
#else
	            - e->globalY())
#endif
	           / winGeometry.height();
	yaw += dx * 3.14f / 3.f;
	pitch += dy * 3.14f / 3.f;
	QCursor::setPos(winGeometry.x() + winGeometry.width() / 2,
	                winGeometry.y() + winGeometry.height() / 2);
}

void FPSCamera::updateLookAt(float frameTiming,
                             GamepadHandler const& gamepadHandler)
{
	// set gamepad input
	if(gamepadHandler.isEnabled())
	{
		auto oldGamepadVel = gamepadVel;
		oldGamepadVel.setX(static_cast<int>(oldGamepadVel.x() > 0)
		                   - static_cast<int>(oldGamepadVel.x() < 0));
		oldGamepadVel.setY(static_cast<int>(oldGamepadVel.y() > 0)
		                   - static_cast<int>(oldGamepadVel.y() < 0));
		oldGamepadVel.setZ(static_cast<int>(oldGamepadVel.z() > 0)
		                   - static_cast<int>(oldGamepadVel.z() < 0));
		float yVal = gamepadHandler.getTrigger(Side::RIGHT)
		             - gamepadHandler.getTrigger(Side::LEFT);
		gamepadVel = {gamepadHandler.getJoystick(Side::LEFT).x(), yVal,
		              gamepadHandler.getJoystick(Side::LEFT).y()};
		auto signOfGamepadVel = gamepadVel;
		signOfGamepadVel.setX(static_cast<int>(signOfGamepadVel.x() > 0)
		                      - static_cast<int>(signOfGamepadVel.x() < 0));
		signOfGamepadVel.setY(static_cast<int>(signOfGamepadVel.y() > 0)
		                      - static_cast<int>(signOfGamepadVel.y() < 0));
		signOfGamepadVel.setZ(static_cast<int>(signOfGamepadVel.z() > 0)
		                      - static_cast<int>(signOfGamepadVel.z() < 0));
	}
	else
	{
		gamepadVel = {};
	}

	// apply gamepad and keyboard controls
	auto vec(utils::transformPosition(noTrans(getView()).inverted(),
	                                  negVel + posVel + gamepadVel));
	for(unsigned int i(0); i < 3; ++i)
	{
		position[i] += frameTiming * vec[i];
	}
	setView(position, getLookDirection(), QVector3D(0.f, 0.f, 1.f));
}

QVector3D FPSCamera::getLookDirection() const
{
	return {-cosf(yaw) * cosf(pitch), -sinf(yaw) * cosf(pitch), sinf(pitch)};
}
