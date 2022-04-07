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

#ifndef GAMEPADHANDLER_HPP
#define GAMEPADHANDLER_HPP

#include <QSettings>
#include <QVector2D>
#include <queue>

#include "utils.hpp"

class QGamepad;

class GamepadHandler : public QObject
{
	Q_OBJECT

  public: // public types
	enum class EventType
	{
		NONE,
		BUTTON_PRESSED,
		BUTTON_UNPRESSED
	};

	enum class Button
	{
		NONE   = 0x0,
		A      = 0x1,
		B      = 0x2,
		X      = 0x4,
		Y      = 0x8,
		L1     = 0x10,
		R1     = 0x20,
		L3     = 0x40,
		R3     = 0x80,
		UP     = 0x100,
		DOWN   = 0x200,
		LEFT   = 0x400,
		RIGHT  = 0x800,
		CENTER = 0x1000,
		SELECT = 0x2000,
		START  = 0x4000,
		GUIDE  = 0x8000,
	};

	struct Event
	{
		Event() = default;
		Event(EventType type, Button button)
		    : type(type)
		    , button(button){};
		EventType type = EventType::NONE;
		Button button  = Button::NONE;
	};

  public:
	GamepadHandler();
	bool isEnabled() const { return gamepad != nullptr; };
	QVector2D getJoystick(Side side) const;
	double getTrigger(Side side) const;
	bool pollEvent(Event& e);

	// deviceid, name
	static QList<QPair<int, QString>> getConnectedGamepads(bool noEmptyName
	                                                       = false);

  private slots:
	void updateGamepad();

  private:
	void setupGamepadConnections();

	int desiredDeviceId
	    = QSettings().value("controls/gamepad").toString().toInt();

	QGamepad* gamepad = nullptr;
	QString gamepadName;

	std::queue<Event> events;

	// calibration
	float deadzone = 0.05f;
};

#endif // GAMEPADHANDLER_HPP
