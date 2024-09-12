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

#include "gl/GLCullFaceSet.hpp"

#include "gl/GLHandler.hpp"

GLCullFaceSet::CullFaceState::CullFaceState()
    : CullFaceState(GL_BACK)
{
}

GLCullFaceSet::CullFaceState::CullFaceState(int faceToCull)
    : CullFaceState(faceToCull, GL_CCW)
{
}

GLCullFaceSet::CullFaceState& GLCullFaceSet::globalState()
{
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glCullFace.xhtml
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glFrontFace.xhtml
	static CullFaceState globalState(GL_BACK, GL_CCW);
	return globalState;
}

GLCullFaceSet::GLCullFaceSet(CullFaceState const& stateSet)
    : revertSet(globalState())
{
	GLHandler::glf().glCullFace(stateSet.faceToCull);
	GLHandler::glf().glFrontFace(stateSet.frontFaceWindingOrder);

	globalState() = stateSet;
}

GLCullFaceSet::~GLCullFaceSet()
{
	GLHandler::glf().glCullFace(revertSet.faceToCull);
	GLHandler::glf().glFrontFace(revertSet.frontFaceWindingOrder);

	globalState() = revertSet;
}

GLCullFaceSet::CullFaceState GLCullFaceSet::getGlobalState()
{
	return globalState();
}

GLCullFaceSet::CullFaceState GLCullFaceSet::getTrueGlobalState()
{
	CullFaceState result;
	GLHandler::glf().glGetIntegerv(GL_CULL_FACE_MODE, &result.faceToCull);
	GLHandler::glf().glGetIntegerv(GL_FRONT_FACE,
	                               &result.frontFaceWindingOrder);
	return result;
}

#include "Logger.hpp"

void GLCullFaceSet::printDifferences(CullFaceState const& globalState0,
                                     CullFaceState const& globalState1)
{
	qDebug() << "OpenGL global blend state differences :";
	const Logger::NoFormatGuard f;
	if(globalState0.faceToCull != globalState1.faceToCull)
	{
		qDebug() << "faceToCull" << globalState0.faceToCull << "vs"
		         << globalState1.faceToCull << '\n';
	}
	if(globalState0.frontFaceWindingOrder != globalState1.frontFaceWindingOrder)
	{
		qDebug() << "frontFaceWindingOrder"
		         << globalState0.frontFaceWindingOrder << "vs"
		         << globalState1.frontFaceWindingOrder << '\n';
	}
}
