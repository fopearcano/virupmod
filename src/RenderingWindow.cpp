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

#include "RenderingWindow.hpp"

#include <QGuiApplication>
#include <QKeyEvent>
#include <QScreen>

RenderingWindow::RenderingWindow(unsigned int id)
    : id(id)
{
	if(id > 0)
	{
		setTitle(PROJECT_NAME + tr(" - Subwindow ") + QString::number(id));
	}
	params.fromStr(
	    QSettings().value("window/windefinitions").toStringList().at(id));

	setSurfaceType(QSurface::OpenGLSurface);

	QSurfaceFormat format;
	format.setDepthBufferSize(24);
	format.setStencilBufferSize(8);
	format.setVersion(4, 2);
	format.setSwapInterval(QSettings().value("window/vsync").toBool() ? 1 : 0);
	format.setSwapBehavior(QSurfaceFormat::TripleBuffer);
	setFormat(format);

	updateAngleShiftMat();
}

bool RenderingWindow::isFullscreen() const
{
	return params.fullscreen;
}

void RenderingWindow::setFullscreen(bool fullscreen)
{
	params.fullscreen = fullscreen;
	saveParams();
	if(fullscreen)
	{
		QRect screenGeometry(screen()->geometry());
		if(params.screenname != "")
		{
			for(auto s : QGuiApplication::screens())
			{
				if(s->name() == params.screenname)
				{
					screenGeometry = s->geometry();
					break;
				}
			}
		}
		setFlags(flags() | Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
		setGeometry(screenGeometry);
		showFullScreen();
	}
	else
	{
		setFlags(flags() & ~Qt::CustomizeWindowHint & ~Qt::FramelessWindowHint);
		show();
		resize(params.width, params.height);
	}
}

double RenderingWindow::getHorizontalAngleShift() const
{
	return params.horizontalAngleShift;
}

double RenderingWindow::getVerticalAngleShift() const
{
	return params.verticalAngleShift;
}

void RenderingWindow::setHorizontalAngleShift(double angleShift)
{
	params.horizontalAngleShift = angleShift;
	updateAngleShiftMat();
	saveParams();
}

void RenderingWindow::setVerticalAngleShift(double angleShift)
{
	params.verticalAngleShift = angleShift;
	updateAngleShiftMat();
	saveParams();
}

void RenderingWindow::toggleFullscreen()
{
	setFullscreen(!isFullscreen());
}

void RenderingWindow::keyPressEvent(QKeyEvent* e)
{
	QString modifier;
	QString key;

	if((e->modifiers() & Qt::ShiftModifier) != 0u)
	{
		modifier += "Shift+";
	}
	if((e->modifiers() & Qt::ControlModifier) != 0u)
	{
		modifier += "Ctrl+";
	}
	if((e->modifiers() & Qt::AltModifier) != 0u)
	{
		modifier += "Alt+";
	}
	if((e->modifiers() & Qt::MetaModifier) != 0u)
	{
		modifier += "Meta+";
	}

	key = QKeySequence(e->key()).toString();

	QKeySequence ks(modifier + key);
	actionEvent(inputManager[ks], true);

	if(!PythonQtHandler::isSupported())
	{
		return;
	}

	QString pyKeyEvent("QKeyEvent(");
	pyKeyEvent += QString::number(e->type()) + ",";
	pyKeyEvent += QString::number(e->key()) + ",";
	pyKeyEvent += QString::number(e->modifiers()) + ",";
	pyKeyEvent += QString::number(e->nativeScanCode()) + ",";
	pyKeyEvent += QString::number(e->nativeVirtualKey()) + ",";
	pyKeyEvent += QString::number(e->nativeModifiers()) + ",";
	if(e->key() != Qt::Key_Return && e->key() != Qt::Key_Enter)
	{
		pyKeyEvent += "\"" + e->text().replace('"', "\\\"") + "\",";
	}
	else
	{
		pyKeyEvent += R"("\n",)";
	}
	pyKeyEvent += e->isAutoRepeat() ? "True," : "False,";
	pyKeyEvent += QString::number(e->count()) + ")";

	PythonQtHandler::evalScript(
	    "if \"keyPressEvent\" in dir():\n\tkeyPressEvent(" + pyKeyEvent + ")");
}

void RenderingWindow::keyReleaseEvent(QKeyEvent* e)
{
	QString modifier;
	QString key;

	if((e->modifiers() & Qt::ShiftModifier) != 0u)
	{
		modifier += "Shift+";
	}
	if((e->modifiers() & Qt::ControlModifier) != 0u)
	{
		modifier += "Ctrl+";
	}
	if((e->modifiers() & Qt::AltModifier) != 0u)
	{
		modifier += "Alt+";
	}
	if((e->modifiers() & Qt::MetaModifier) != 0u)
	{
		modifier += "Meta+";
	}

	key = QKeySequence(e->key()).toString();

	QKeySequence ks(modifier + key);
	actionEvent(inputManager[ks], false);

	if(!PythonQtHandler::isSupported())
	{
		return;
	}

	QString pyKeyEvent("QKeyEvent(");
	pyKeyEvent += QString::number(e->type()) + ",";
	pyKeyEvent += QString::number(e->key()) + ",";
	pyKeyEvent += QString::number(e->modifiers()) + ",";
	pyKeyEvent += QString::number(e->nativeScanCode()) + ",";
	pyKeyEvent += QString::number(e->nativeVirtualKey()) + ",";
	pyKeyEvent += QString::number(e->nativeModifiers()) + ",";
	if(e->key() != Qt::Key_Return && e->key() != Qt::Key_Enter)
	{
		pyKeyEvent += "\"" + e->text().replace('"', "\\\"") + "\",";
	}
	else
	{
		pyKeyEvent += R"("\n",)";
	}
	pyKeyEvent += e->isAutoRepeat() ? "True," : "False,";
	pyKeyEvent += QString::number(e->count()) + ")";

	PythonQtHandler::evalScript(
	    "if \"keyReleaseEvent\" in dir():\n\tkeyReleaseEvent(" + pyKeyEvent
	    + ")");
}

void RenderingWindow::actionEvent(BaseInputManager::Action a, bool pressed)
{
	if(!pressed)
	{
		return;
	}

	if(a.id == "togglefullscreen")
	{
		toggleFullscreen();
	}
	else if(a.id == "quit")
	{
		close();
	}
}

void RenderingWindow::resizeEvent(QResizeEvent* /*ev*/)
{
	if(!isFullscreen())
	{
		params.width  = size().width();
		params.height = size().height();
		saveParams();
	}
}

void RenderingWindow::updateAngleShiftMat()
{
	angleShiftMat = QMatrix4x4();
	angleShiftMat.rotate(params.verticalAngleShift, QVector3D(-1.f, 0.f, 0.f));
	angleShiftMat.rotate(params.horizontalAngleShift, QVector3D(0.f, 1.f, 0.f));
}

void RenderingWindow::saveParams() const
{
	QStringList allParams(
	    QSettings().value("window/windefinitions").toStringList());
	allParams[id] = params.toStr();
	QSettings().setValue("window/windefinitions", allParams);
}
