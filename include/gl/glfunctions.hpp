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

#ifndef OPENGL_MAJOR_VERSION
#define OPENGL_MAJOR_VERSION 4
#endif

#ifndef OPENGL_MINOR_VERSION
#define OPENGL_MINOR_VERSION 2
#endif

#ifndef OPENGL_PROFILE
#define OPENGL_PROFILE Core
#endif

#define CAT(x, y) CAT_(x, y)
#define CAT_(x, y) x##y

#define QOPENGLFUNCTIONS                                              \
	CAT(CAT(CAT(CAT(CAT(QOpenGLFunctions_, OPENGL_MAJOR_VERSION), _), \
	            OPENGL_MINOR_VERSION),                                \
	        _),                                                       \
	    Core)

#define QSURFACEFORMATPROFILE CAT(OPENGL_PROFILE, Profile)

#define QUOTEME(M) #M
#define INCLUDE_FILE(M) QUOTEME(M)

#include INCLUDE_FILE(QOPENGLFUNCTIONS)

typedef QOPENGLFUNCTIONS OpenGLFunctions;

namespace gl
{
const unsigned int majorVersion = OPENGL_MAJOR_VERSION;
const unsigned int minorVersion = OPENGL_MINOR_VERSION;
const QSurfaceFormat::OpenGLContextProfile profile
    = QSurfaceFormat::QSURFACEFORMATPROFILE;
} // namespace gl

#endif // GLFUNCTIONS_HPP
