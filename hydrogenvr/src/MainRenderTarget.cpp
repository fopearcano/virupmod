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

#include "MainRenderTarget.hpp"

MainRenderTarget::MainRenderTarget(unsigned int width, unsigned int height,
                                   unsigned int samples, Projection projection)
    : sceneTarget(constructSceneTarget(width, height, samples, projection))
    , postProcessingTargets({GLFramebufferObject(GLTexture::Tex2DProperties(
                                 width, height, GL_RGBA32F)),
                             GLFramebufferObject(GLTexture::Tex2DProperties(
                                 width, height, GL_RGBA32F))})
{
	sceneTarget.setName("Main Scene");
	postProcessingTargets[0].setName("Post-processing 0");
	postProcessingTargets[1].setName("Post-processing 1");
}

GLFramebufferObject MainRenderTarget::constructSceneTarget(
    unsigned int width, unsigned int height, unsigned int samples,
    Projection projection)
{
	if(projection == Projection::DEFAULT)
	{
		if(samples > 1)
		{
			return GLFramebufferObject(GLTexture::TexMultisampleProperties(
			    width, height, samples, GL_RGBA32F));
		}
		return GLFramebufferObject(
		    GLTexture::Tex2DProperties(width, height, GL_RGBA32F));
	}
	if(projection == Projection::VR180L || projection == Projection::VR180R)
	{
		return GLFramebufferObject(
		    GLTexture::TexCubemapProperties(width * 4 / 3, GL_RGBA32F));
	}
	return GLFramebufferObject(
	    GLTexture::TexCubemapProperties(width * 2 / 3, GL_RGBA32F));
}

MainRenderTarget::Projection MainRenderTarget::strToProj(QString const& str)
{
	if(str == "panorama360")
	{
		return MainRenderTarget::Projection::PANORAMA360;
	}
	if(str == "vr180l")
	{
		return MainRenderTarget::Projection::VR180L;
	}
	if(str == "vr180r")
	{
		return MainRenderTarget::Projection::VR180R;
	}
	if(str == "vr180")
	{
		return MainRenderTarget::Projection::VR180;
	}
	if(str == "domemaster180")
	{
		return MainRenderTarget::Projection::DOMEMASTER180;
	}
	return MainRenderTarget::Projection::DEFAULT;
}

QString MainRenderTarget::projToStr(MainRenderTarget::Projection proj)
{
	switch(proj)
	{
		case Projection::PANORAMA360:
			return "panorama360";
		case Projection::VR180L:
			return "vr180l";
		case Projection::VR180R:
			return "vr180r";
		case Projection::VR180:
			return "vr180";
		case Projection::DOMEMASTER180:
			return "domemaster180";
		default:
			return "default";
	}
}
