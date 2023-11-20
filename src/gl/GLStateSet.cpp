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

#include "gl/GLStateSet.hpp"

#include "gl/GLHandler.hpp"

std::unordered_map<int, bool>& GLStateSet::globalState()
{
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glEnable.xhtml
	static std::unordered_map<int, bool> globalState;
	globalState[GL_BLEND]                         = false;
	globalState[GL_CLIP_DISTANCE0]                = false;
	globalState[GL_CLIP_DISTANCE1]                = false;
	globalState[GL_CLIP_DISTANCE2]                = false;
	globalState[GL_CLIP_DISTANCE3]                = false;
	globalState[GL_CLIP_DISTANCE4]                = false;
	globalState[GL_CLIP_DISTANCE5]                = false;
	globalState[GL_CLIP_DISTANCE6]                = false;
	globalState[GL_CLIP_DISTANCE7]                = false;
	globalState[GL_COLOR_LOGIC_OP]                = false;
	globalState[GL_CULL_FACE]                     = false;
	globalState[GL_DEBUG_OUTPUT]                  = false;
	globalState[GL_DEBUG_OUTPUT_SYNCHRONOUS]      = false;
	globalState[GL_DEPTH_CLAMP]                   = false;
	globalState[GL_DEPTH_TEST]                    = false;
	globalState[GL_DITHER]                        = true;
	globalState[GL_FRAMEBUFFER_SRGB]              = false;
	globalState[GL_LINE_SMOOTH]                   = false;
	globalState[GL_MULTISAMPLE]                   = true;
	globalState[GL_POLYGON_OFFSET_FILL]           = false;
	globalState[GL_POLYGON_OFFSET_LINE]           = false;
	globalState[GL_POLYGON_OFFSET_POINT]          = false;
	globalState[GL_POLYGON_SMOOTH]                = false;
	globalState[GL_PRIMITIVE_RESTART]             = false;
	globalState[GL_PRIMITIVE_RESTART_FIXED_INDEX] = false;
	globalState[GL_RASTERIZER_DISCARD]            = false;
	globalState[GL_SAMPLE_ALPHA_TO_COVERAGE]      = false;
	globalState[GL_SAMPLE_ALPHA_TO_ONE]           = false;
	globalState[GL_SAMPLE_COVERAGE]               = false;
	globalState[GL_SAMPLE_SHADING]                = false;
	globalState[GL_SAMPLE_MASK]                   = false;
	globalState[GL_SCISSOR_TEST]                  = false;
	globalState[GL_STENCIL_TEST]                  = false;
	globalState[GL_TEXTURE_CUBE_MAP_SEAMLESS]     = false;
	globalState[GL_PROGRAM_POINT_SIZE]            = false;

	// apply GLHandler defaults
	globalState[GL_CULL_FACE]  = true;
	globalState[GL_DEPTH_TEST] = true;

	return globalState;
}

GLStateSet::GLStateSet(std::unordered_map<int, bool> const& stateSet)
{
	for(auto const& pair : stateSet)
	{
		revertSet[pair.first] = globalState()[pair.first];
		if(pair.second)
		{
			GLHandler::glf().glEnable(pair.first);
		}
		else
		{
			GLHandler::glf().glDisable(pair.first);
		}
		globalState()[pair.first] = pair.second;
	}
}

GLStateSet::~GLStateSet()
{
	for(auto const& pair : revertSet)
	{
		if(pair.second)
		{
			GLHandler::glf().glEnable(pair.first);
		}
		else
		{
			GLHandler::glf().glDisable(pair.first);
		}
		globalState()[pair.first] = pair.second;
	}
}

std::unordered_map<int, bool> GLStateSet::getGlobalState()
{
	return globalState();
}

std::unordered_map<int, bool> GLStateSet::getTrueGlobalState()
{
	std::unordered_map<int, bool> result;
	for(auto const& pair : globalState())
	{
		result[pair.first] = (GLHandler::glf().glIsEnabled(pair.first) != 0u);
	}
	return result;
}

#include "Logger.hpp"

void GLStateSet::printDifferences(
    std::unordered_map<int, bool> const& globalState0,
    std::unordered_map<int, bool> const& globalState1)
{
	qDebug() << "OpenGL global state differences :";
	Logger::NoFormatGuard f;
	for(auto const& pair : globalState())
	{
		if(globalState0.at(pair.first) != globalState1.at(pair.first))
		{
			qDebug() << pair.first << globalState0.at(pair.first) << "vs"
			         << globalState1.at(pair.first) << '\n';
		}
	}
}
