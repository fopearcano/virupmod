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

GLStateSet::GlobalState::GlobalState()
{
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glEnable.xhtml
	map[GL_BLEND]                         = false;
	map[GL_CLIP_DISTANCE0]                = false;
	map[GL_CLIP_DISTANCE1]                = false;
	map[GL_CLIP_DISTANCE2]                = false;
	map[GL_CLIP_DISTANCE3]                = false;
	map[GL_CLIP_DISTANCE4]                = false;
	map[GL_CLIP_DISTANCE5]                = false;
	map[GL_CLIP_DISTANCE6]                = false;
	map[GL_CLIP_DISTANCE7]                = false;
	map[GL_COLOR_LOGIC_OP]                = false;
	map[GL_CULL_FACE]                     = false;
	map[GL_DEBUG_OUTPUT]                  = false;
	map[GL_DEBUG_OUTPUT_SYNCHRONOUS]      = false;
	map[GL_DEPTH_CLAMP]                   = false;
	map[GL_DEPTH_TEST]                    = false;
	map[GL_DITHER]                        = true;
	map[GL_FRAMEBUFFER_SRGB]              = false;
	map[GL_LINE_SMOOTH]                   = false;
	map[GL_MULTISAMPLE]                   = true;
	map[GL_POLYGON_OFFSET_FILL]           = false;
	map[GL_POLYGON_OFFSET_LINE]           = false;
	map[GL_POLYGON_OFFSET_POINT]          = false;
	map[GL_POLYGON_SMOOTH]                = false;
	map[GL_PRIMITIVE_RESTART]             = false;
	map[GL_PRIMITIVE_RESTART_FIXED_INDEX] = false;
	map[GL_RASTERIZER_DISCARD]            = false;
	map[GL_SAMPLE_ALPHA_TO_COVERAGE]      = false;
	map[GL_SAMPLE_ALPHA_TO_ONE]           = false;
	map[GL_SAMPLE_COVERAGE]               = false;
	map[GL_SAMPLE_SHADING]                = false;
	map[GL_SAMPLE_MASK]                   = false;
	map[GL_SCISSOR_TEST]                  = false;
	map[GL_STENCIL_TEST]                  = false;
	map[GL_TEXTURE_CUBE_MAP_SEAMLESS]     = false;
	map[GL_PROGRAM_POINT_SIZE]            = false;

	// apply GLHandler defaults
	map[GL_CULL_FACE]  = true;
	map[GL_DEPTH_TEST] = true;
}

std::unordered_map<int, bool>& GLStateSet::globalState()
{
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glEnable.xhtml
	static GlobalState globalState;

	return globalState.map;
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
	const Logger::NoFormatGuard f;
	for(auto const& pair : globalState())
	{
		if(globalState0.at(pair.first) != globalState1.at(pair.first))
		{
			qDebug() << pair.first << globalState0.at(pair.first) << "vs"
			         << globalState1.at(pair.first) << '\n';
		}
	}
}
