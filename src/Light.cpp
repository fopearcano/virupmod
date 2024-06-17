/*
    Copyright (C) 2019 Florian Cabot <florian.cabot@hotmail.fr>

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

#include "Light.hpp"
#include "Primitives.hpp"

unsigned int Light::getResolution()
{
	return 1u << (9 + QSettings().value("graphics/shadowsquality").toUInt());
}

Light::Light(QVector3D const& direction, float boundingSphereRadius)
    : color(1.0, 1.0, 1.0)
    , ambiantFactor(0.05f)
    , shadowMap(GLTexture::Tex2DProperties(getResolution(), getResolution(),
                                           GL_DEPTH_COMPONENT32))
    , shadowShader("shadow")
    , def("default")
{
	shadowMap.setColorAttachmentName("ShadowMap");
	Primitives::setAsUnitSphere(mesh, def, 100, 100);

	bias = {};
	bias.translate(0.5f, 0.5f, 0.5f);
	bias.scale(0.5);

	setDirection(direction);
	setBoundingSphereRadius(boundingSphereRadius);
}

void Light::setCenter(QVector3D const& center)
{
	this->center = center;

	view = {};
	view.lookAt(center, center + direction, QVector3D(0.f, 0.f, 1.f));
}

void Light::setDirection(QVector3D const& direction)
{
	this->direction = direction.normalized();

	view = {};
	view.lookAt(center, center + direction, QVector3D(0.f, 0.f, 1.f));
}

void Light::setBoundingSphereRadius(float boundingSphereRadius)
{
	this->boundingSphereRadius = boundingSphereRadius;

	proj = {};
	proj.ortho(-1.f * boundingSphereRadius, boundingSphereRadius,
	           -1.f * boundingSphereRadius, boundingSphereRadius,
	           -1.f * boundingSphereRadius, boundingSphereRadius);
}

QMatrix4x4 Light::getTransformation(QMatrix4x4 const& model, bool biased) const
{
	if(biased)
	{
		return bias * proj * view * model;
	}
	return proj * view * model;
}

void Light::setUpShader(GLShaderProgram const& shader,
                        QMatrix4x4 const& model) const
{
	QVector3D relDir = QVector3D(model.inverted() * QVector4D(direction, 0.f));
	shader.setUniform("lightDirection", relDir.normalized());
	shader.setUniform("lightColor", color);
	shader.setUniform("lightAmbiantFactor", ambiantFactor);
	shader.setUniform("lightspace", getTransformation(model, true));
	shader.setUniform("boundingSphereRadius", boundingSphereRadius);
}

GLTexture const& Light::getShadowMap() const
{
	return shadowMap.getColorAttachmentTexture();
}

void Light::generateShadowMap(std::vector<GLMesh const*> const& meshes,
                              std::vector<QMatrix4x4> const& models,
                              QMatrix4x4 const& model) const
{
	// see third comment :
	// https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
	GLStateSet glState({{GL_CULL_FACE, false}});
	GLHandler::beginRendering(GL_DEPTH_BUFFER_BIT, shadowMap);
	QMatrix4x4 lightSpace(getTransformation(model));
	for(unsigned int i(0); i < meshes.size(); ++i)
	{
		shadowShader.setUniform("camera", lightSpace * models[i]);
		meshes[i]->render();
	}
}

void Light::render(float angularSizeRad)
{
	def.setUniform("color", color);

	QMatrix4x4 model;
	model.translate(-direction.normalized());
	model.scale(tan(0.5 * angularSizeRad));

	GLStateSet glState({{GL_DEPTH_TEST, false}});
	GLHandler::setUpRender(def, model, GLHandler::GeometricSpace::SKYBOX);
	mesh.render();
}
