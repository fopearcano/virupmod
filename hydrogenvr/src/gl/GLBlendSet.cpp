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

#include "gl/GLBlendSet.hpp"

#include "gl/GLHandler.hpp"

GLBlendSet::BlendState::BlendState()
    : BlendState(GL_SRC_ALPHA)
{
}

GLBlendSet::BlendState::BlendState(int blendfuncSfactor)
    : BlendState(blendfuncSfactor, GL_ONE_MINUS_SRC_ALPHA)
{
}

GLBlendSet::BlendState& GLBlendSet::globalState()
{
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBlendFunc.xhtml
	// https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glDepthMask.xml
	static BlendState globalState(GL_ONE, GL_ZERO, true);
	return globalState;
}

GLBlendSet::GLBlendSet(BlendState const& stateSet)
    : enableBlendStateSet({{GL_BLEND, true}})
    , revertSet(globalState())
{
	GLHandler::glf().glDepthMask(stateSet.depthMask ? GL_TRUE : GL_FALSE);
	GLHandler::glf().glBlendFunc(stateSet.blendfuncSfactor,
	                             stateSet.blendfuncDfactor);

	globalState() = stateSet;
}

GLBlendSet::~GLBlendSet()
{
	GLHandler::glf().glDepthMask(revertSet.depthMask ? GL_TRUE : GL_FALSE);
	GLHandler::glf().glBlendFunc(revertSet.blendfuncSfactor,
	                             revertSet.blendfuncDfactor);
	globalState() = revertSet;
}

GLBlendSet::BlendState GLBlendSet::getGlobalState()
{
	return globalState();
}

GLBlendSet::BlendState GLBlendSet::getTrueGlobalState()
{
	BlendState result;
	GLHandler::glf().glGetIntegerv(GL_BLEND_SRC, &result.blendfuncSfactor);
	GLHandler::glf().glGetIntegerv(GL_BLEND_DST, &result.blendfuncDfactor);
	GLboolean b = 0;
	GLHandler::glf().glGetBooleanv(GL_DEPTH_WRITEMASK, &b);
	result.depthMask = (b != 0u);
	return result;
}

#include "Logger.hpp"

void GLBlendSet::printDifferences(BlendState const& globalState0,
                                  BlendState const& globalState1)
{
	qDebug() << "OpenGL global blend state differences :";
	const Logger::NoFormatGuard f;
	if(globalState0.blendfuncSfactor != globalState1.blendfuncSfactor)
	{
		qDebug() << "blendfuncSfactor" << globalState0.blendfuncSfactor << "vs"
		         << globalState1.blendfuncSfactor << '\n';
	}
	if(globalState0.blendfuncDfactor != globalState1.blendfuncDfactor)
	{
		qDebug() << "blendfuncDfactor" << globalState0.blendfuncDfactor << "vs"
		         << globalState1.blendfuncDfactor << '\n';
	}
	if(globalState0.depthMask != globalState1.depthMask)
	{
		qDebug() << "depthMask" << globalState0.depthMask << "vs"
		         << globalState1.depthMask << '\n';
	}
}
