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

#ifndef TEXTUREDISPLAYWINDOW_HPP
#define TEXTUREDISPLAYWINDOW_HPP

#include <QSettings>
#include <QWindow>

#include "gl/GLHandler.hpp"

/** A window used to render a texture to.
 * Embed it in a QWidget to put in inside another window.
 */
class TextureDisplayWindow : public QWindow
{
  public:
	TextureDisplayWindow()
	{
		setSurfaceType(QSurface::OpenGLSurface);

		QSurfaceFormat format;
		format.setDepthBufferSize(24);
		format.setStencilBufferSize(8);
		format.setVersion(gl::majorVersion, gl::minorVersion);
		format.setSwapInterval(QSettings().value("window/vsync").toBool() ? 1
		                                                                  : 0);
		format.setSwapBehavior(QSurfaceFormat::TripleBuffer);
		format.setProfile(gl::profile);
		setFormat(format);
	}
};

#endif // TEXTUREDISPLAYWINDOW_HPP
