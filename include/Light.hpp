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

#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "Primitives.hpp"
#include "gl/GLHandler.hpp"

// for now directional only
class Light
{
  public:
	Light(QVector3D const& direction = {-1.f, 0.f, 0.f},
	      float boundingSphereRadius = 1.f);
	QVector3D getCenter() const { return center; };
	void setCenter(QVector3D const& center);
	QVector3D getDirection() const { return direction; };
	void setDirection(QVector3D const& direction);
	float getBoundingSphereRadius() const { return boundingSphereRadius; };
	void setBoundingSphereRadius(float boundingSphereRadius);

	QMatrix4x4 getTransformation(QMatrix4x4 const& model,
	                             bool biased = false) const;
	void setUpShader(GLShaderProgram const& shader,
	                 QMatrix4x4 const& model) const;
	GLTexture const& getShadowMap() const;
	void generateShadowMap(std::vector<GLMesh const*> const& meshes,
	                       std::vector<QMatrix4x4> const& models,
	                       QMatrix4x4 const& model = {}) const;
	// renders a bright sphere at infinity, default is angular size of the sun
	void render(float angularSizeRad = 0.542f * M_PI / 180.f);

	QVector3D color; // linear RGB in luminance units
	float ambiantFactor;

  private:
	GLFramebufferObject shadowMap;
	GLShaderProgram shadowShader;

	QVector3D center;
	QVector3D direction;
	float boundingSphereRadius = 1.f;
	QMatrix4x4 bias;
	QMatrix4x4 proj;
	QMatrix4x4 view;

	static unsigned int getResolution();

	// for rendering
	GLShaderProgram def;
	GLMesh mesh;
};

#endif // LIGHT_HPP
