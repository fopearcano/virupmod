/*
    Copyright (C) 2019 Florian Cabot <florian.cabot@epfl.ch>

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

#include "Billboard.hpp"

#include "camera/BasicCamera.hpp"

Billboard::Billboard(const char* texPath)
    : Billboard(texPath, GLShaderProgram("billboard"))
{
}
Billboard::Billboard(QImage const& image)
    : Billboard(image, GLShaderProgram("billboard"))
{
}

Billboard::Billboard(GLTexture&& texture)
    : Billboard(std::move(texture), GLShaderProgram("billboard"))
{
}

Billboard::Billboard(const char* texPath, GLShaderProgram&& shader)
    : Billboard(QImage(texPath).mirrored(), std::move(shader))
{
}

Billboard::Billboard(QImage const& image, GLShaderProgram&& shader)
    : Billboard(GLTexture(image), std::move(shader))
{
}

Billboard::Billboard(GLTexture&& texture, GLShaderProgram&& shader)
    : tex(std::move(texture))
    , shader(std::move(shader))
{
	Primitives::setAsQuad(quad, this->shader, PrimitiveType::TRIANGLE_STRIP);

	auto s(tex.getSize());
	QSize size(s[0], s[1]);

	if(size.width() > size.height())
	{
		aspectratio.scale(1.f,
		                  static_cast<float>(size.height()) / size.width());
	}
	else
	{
		aspectratio.scale(static_cast<float>(size.width()) / size.height(),
		                  1.f);
	}
}

void Billboard::render(BasicCamera const& camera)
{
	QVector3D hmdPos = utils::transformPosition(
	    camera.hmdSpaceToWorldTransform().inverted(), position);
	QMatrix4x4 model;
	model.translate(hmdPos);
	model.scale(width / camera.getEyeDistanceFactor());
	GLBlendSet glBlend(GLBlendSet::BlendState{});
	GLHandler::useTextures({&tex});
	GLHandler::setUpRender(shader, model * aspectratio,
	                       GLHandler::GeometricSpace::HMD);
	quad.render(PrimitiveType::TRIANGLE_STRIP);
}
