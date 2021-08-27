/*
    Copyright (C) 2020 Florian Cabot <florian.cabot@hotmail.fr>

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

#ifndef MAINRENDERTARGET_HPP
#define MAINRENDERTARGET_HPP

#include "gl/GLHandler.hpp"

class MainRenderTarget
{
  public:
	enum class Projection
	{
		DEFAULT       = 0,
		PANORAMA360   = 1,
		VR180L        = 2,
		VR180R        = 3,
		VR180         = 4,
		DOMEMASTER180 = 5,
	};

	MainRenderTarget(unsigned int width, unsigned int height,
	                 unsigned int samples, Projection projection);

	static GLFramebufferObject constructSceneTarget(unsigned int width,
	                                                unsigned int height,
	                                                unsigned int samples,
	                                                Projection projection);
	static MainRenderTarget::Projection strToProj(QString const& str);
	static QString projToStr(MainRenderTarget::Projection proj);

  public:
	GLFramebufferObject sceneTarget;
	std::array<GLFramebufferObject, 2> postProcessingTargets;
};

#endif // MAINRENDERTARGET_HPP
