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

#include "GamepadHandler.hpp"

#include <QtDebug>
#ifdef QT5_GAMEPAD
#include <QtGamepad/QGamepad>
#include <QtGamepad/QGamepadManager>
#endif

GamepadHandler::GamepadHandler()
{
#ifdef QT5_GAMEPAD
	if(desiredDeviceId == -1)
	{
		return;
	}

	updateGamepad();

	QObject::connect(QGamepadManager::instance(),
	                 &QGamepadManager::connectedGamepadsChanged, this,
	                 &GamepadHandler::updateGamepad);
#endif
}

QVector2D GamepadHandler::getJoystick(Side side) const
{
#ifdef QT5_GAMEPAD
	if(!isEnabled() || side == Side::NONE)
	{
		return {};
	}

	QVector2D res;
	if(side == Side::LEFT)
	{
		res = QVector2D(gamepad->axisLeftX(), gamepad->axisLeftY());
	}
	if(side == Side::RIGHT)
	{
		res = QVector2D(gamepad->axisRightX(), gamepad->axisRightY());
	}

	if(deadzone * deadzone < res.lengthSquared())
	{
		return res;
	}
#else
	(void) side;
#endif
	return {};
}

double GamepadHandler::getTrigger(Side side) const
{
#ifdef QT5_GAMEPAD
	if(!isEnabled() || side == Side::NONE)
	{
		return 0.0;
	}

	if(side == Side::LEFT && gamepad->buttonL2() > deadzone)
	{
		return gamepad->buttonL2();
	}
	if(side == Side::RIGHT && gamepad->buttonR2() > deadzone)
	{
		return gamepad->buttonR2();
	}
#else
	(void) side;
#endif
	return 0.0;
}

bool GamepadHandler::pollEvent(Event& e)
{
	if(events.empty())
	{
		return false;
	}
	e = events.front();
	events.pop();
	return true;
}

#ifdef Q_OS_WIN
#include <QApplication>
#include <QWindow>
#endif
QList<QPair<int, QString>>
    GamepadHandler::getConnectedGamepads(bool noEmptyName)
{
#ifdef QT5_GAMEPAD
	auto gamepadManager = QGamepadManager::instance();

#ifdef Q_OS_WIN
	// dirty Windows workaround to delay fetching of controllers :
	// https://stackoverflow.com/questions/62668629/qgamepadmanager-connecteddevices-empty-but-windows-detects-gamepad
	auto wnd = new QWindow;
	wnd->show();
	delete wnd;
	QApplication::processEvents();
#endif
#endif
	QList<QPair<int, QString>> res;
#ifdef QT5_GAMEPAD
	unsigned int i(1);
	for(auto deviceId : gamepadManager->connectedGamepads())
	{
		auto name = gamepadManager->gamepadName(deviceId);
		if(name.isEmpty() && noEmptyName)
		{
			name = "Gamepad " + QString::number(i);
		}
		res << QPair<int, QString>{deviceId, name};
		++i;
	}
#else
	(void) noEmptyName;
#endif
	return res;
}

void GamepadHandler::updateGamepad()
{
#ifdef QT5_GAMEPAD
	bool previouslyEnabled = isEnabled();
	delete gamepad;
	gamepad          = nullptr;
	bool noEmptyName = false;
#ifdef Q_OS_WIN
	// usually names never get recovered on Windows
	noEmptyName = true;
#endif
	for(auto const& pair : getConnectedGamepads(noEmptyName))
	{
		if(pair.first == desiredDeviceId)
		{
			gamepad = new QGamepad(pair.first);
			setupGamepadConnections();
			if(!previouslyEnabled)
			{
				if(gamepad->name() != "")
				{
					qDebug() << gamepad->name() + " connected...";
					gamepadName = gamepad->name();
				}
				else if(pair.second != "")
				{
					qDebug() << pair.second + " connected...";
					gamepadName = pair.second;
				}
				else // wait for name to be non-empty
				{
					auto const connection = new QMetaObject::Connection;
					*connection = connect(gamepad, &QGamepad::nameChanged,
					                      [this, connection](QString name) {
						                      qDebug()
						                          << name + " connected...";
						                      gamepadName = gamepad->name();
						                      QObject::disconnect(*connection);
						                      delete connection;
					                      });
				}
			}
			break;
		}
	}

	if(previouslyEnabled)
	{
		qDebug() << gamepadName + " disconnected...";
	}
#endif
}

void GamepadHandler::setupGamepadConnections()
{
#ifdef QT5_GAMEPAD
	connect(gamepad, &QGamepad::buttonAChanged, [this](bool v) {
		events.emplace(v ? EventType::BUTTON_PRESSED
		                 : EventType::BUTTON_UNPRESSED,
		               Button::A);
	});
	connect(gamepad, &QGamepad::buttonBChanged, [this](bool v) {
		events.emplace(v ? EventType::BUTTON_PRESSED
		                 : EventType::BUTTON_UNPRESSED,
		               Button::B);
	});
	connect(gamepad, &QGamepad::buttonXChanged, [this](bool v) {
		events.emplace(v ? EventType::BUTTON_PRESSED
		                 : EventType::BUTTON_UNPRESSED,
		               Button::X);
	});
	connect(gamepad, &QGamepad::buttonYChanged, [this](bool v) {
		events.emplace(v ? EventType::BUTTON_PRESSED
		                 : EventType::BUTTON_UNPRESSED,
		               Button::Y);
	});
	connect(gamepad, &QGamepad::buttonL1Changed, [this](bool v) {
		events.emplace(v ? EventType::BUTTON_PRESSED
		                 : EventType::BUTTON_UNPRESSED,
		               Button::L1);
	});
	connect(gamepad, &QGamepad::buttonR1Changed, [this](bool v) {
		events.emplace(v ? EventType::BUTTON_PRESSED
		                 : EventType::BUTTON_UNPRESSED,
		               Button::R1);
	});
	connect(gamepad, &QGamepad::buttonL3Changed, [this](bool v) {
		events.emplace(v ? EventType::BUTTON_PRESSED
		                 : EventType::BUTTON_UNPRESSED,
		               Button::L3);
	});
	connect(gamepad, &QGamepad::buttonR3Changed, [this](bool v) {
		events.emplace(v ? EventType::BUTTON_PRESSED
		                 : EventType::BUTTON_UNPRESSED,
		               Button::R3);
	});
	connect(gamepad, &QGamepad::buttonUpChanged, [this](bool v) {
		events.emplace(v ? EventType::BUTTON_PRESSED
		                 : EventType::BUTTON_UNPRESSED,
		               Button::UP);
	});
	connect(gamepad, &QGamepad::buttonDownChanged, [this](bool v) {
		events.emplace(v ? EventType::BUTTON_PRESSED
		                 : EventType::BUTTON_UNPRESSED,
		               Button::DOWN);
	});
	connect(gamepad, &QGamepad::buttonLeftChanged, [this](bool v) {
		events.emplace(v ? EventType::BUTTON_PRESSED
		                 : EventType::BUTTON_UNPRESSED,
		               Button::LEFT);
	});
	connect(gamepad, &QGamepad::buttonRightChanged, [this](bool v) {
		events.emplace(v ? EventType::BUTTON_PRESSED
		                 : EventType::BUTTON_UNPRESSED,
		               Button::RIGHT);
	});
	connect(gamepad, &QGamepad::buttonCenterChanged, [this](bool v) {
		events.emplace(v ? EventType::BUTTON_PRESSED
		                 : EventType::BUTTON_UNPRESSED,
		               Button::CENTER);
	});
	connect(gamepad, &QGamepad::buttonSelectChanged, [this](bool v) {
		events.emplace(v ? EventType::BUTTON_PRESSED
		                 : EventType::BUTTON_UNPRESSED,
		               Button::SELECT);
	});
	connect(gamepad, &QGamepad::buttonStartChanged, [this](bool v) {
		events.emplace(v ? EventType::BUTTON_PRESSED
		                 : EventType::BUTTON_UNPRESSED,
		               Button::START);
	});
	connect(gamepad, &QGamepad::buttonGuideChanged, [this](bool v) {
		events.emplace(v ? EventType::BUTTON_PRESSED
		                 : EventType::BUTTON_UNPRESSED,
		               Button::GUIDE);
	});
#endif
}
