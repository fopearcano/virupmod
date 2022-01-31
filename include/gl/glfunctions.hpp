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

#ifndef GLFUNCTIONS_HPP
#define GLFUNCTIONS_HPP

#include <QOpenGLFunctions_4_2_Core>

typedef QOpenGLFunctions_4_2_Core OpenGLFunctions;

namespace gl
{
const unsigned int majorVersion = 4;
const unsigned int minorVersion = 2;
const QSurfaceFormat::OpenGLContextProfile profile
    = QSurfaceFormat::CoreProfile;
} // namespace gl

#endif // GLFUNCTIONS_HPP
