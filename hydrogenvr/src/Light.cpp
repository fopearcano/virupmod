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

QMatrix4x4 Light::getTransformation(bool biased) const
{
	if(biased)
	{
		return bias * proj * view;
	}
	return proj * view;
}

GLTexture const& Light::getShadowMap() const
{
	return shadowMap.getColorAttachmentTexture();
}

void Light::generateShadowMap(std::vector<GLMesh const*> const& meshes,
                              std::vector<QMatrix4x4> const& models) const
{
	// see third comment :
	// https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
	const GLStateSet glState({{GL_CULL_FACE, false}});
	GLHandler::beginRendering(GL_DEPTH_BUFFER_BIT, shadowMap);
	const QMatrix4x4 lightSpace(getTransformation());
	for(unsigned int i(0); i < meshes.size(); ++i)
	{
		shadowShader.setUniform("camera", lightSpace * models[i]);
		shadowShader.use();
		meshes[i]->render();
	}
}

void Light::render(float angularSizeRad)
{
	def.setUniform("color", color);

	QMatrix4x4 model;
	model.translate(-direction.normalized());
	model.scale(tan(0.5 * angularSizeRad));

	const GLStateSet glState({{GL_DEPTH_TEST, false}});
	GLHandler::setUpRender(def, model, GLHandler::GeometricSpace::SKYBOX);
	mesh.render();
}

void Light::setUpShader(GLShaderProgram const& shader,
                        std::vector<Light const*> const& lights)
{
	std::vector<QVector3D> lightDirections;
	std::vector<QVector3D> lightColors;
	std::vector<float> lightAmbiantFactors;
	std::vector<QMatrix4x4> lightspaces;
	std::vector<float> boundingSphereRadii;

	for(const auto* light : lights)
	{
		lightDirections.emplace_back(light->direction.normalized());
		lightColors.emplace_back(light->color);
		lightAmbiantFactors.emplace_back(light->ambiantFactor);
		lightspaces.emplace_back(light->getTransformation(true));
		boundingSphereRadii.emplace_back(light->boundingSphereRadius);
	}

	shader.setUniform("lightDirection", lightDirections.size(),
	                  lightDirections.data());
	shader.setUniform("lightColor", lightColors.size(), lightColors.data());
	shader.setUniform("lightAmbiantFactor", lightAmbiantFactors.size(),
	                  lightAmbiantFactors.data());
	shader.setUniform("lightspace", lightspaces.size(), lightspaces.data());
	shader.setUniform("boundingSphereRadius", boundingSphereRadii.size(),
	                  boundingSphereRadii.data());
}
