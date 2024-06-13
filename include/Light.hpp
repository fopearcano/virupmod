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
	Light();
	QMatrix4x4 getTransformation(float boundingSphereRadius,
	                             QMatrix4x4 const& model,
	                             bool biased = false) const;
	void setUpShader(GLShaderProgram const& shader, float boundingSphereRadius,
	                 QMatrix4x4 const& model) const;
	GLTexture const& getShadowMap() const;
	void generateShadowMap(std::vector<GLMesh const*> const& meshes,
	                       float boundingSphereRadius,
	                       std::vector<QMatrix4x4> const& models,
	                       QMatrix4x4 const& model);
	// renders a bright sphere at infinity, default is angular size of the sun
	void render(float angularSizeRad = 0.542f * M_PI / 180.f);

	QVector3D direction;
	QVector3D color; // linear RGB in luminance units
	float ambiantFactor;

  private:
	GLFramebufferObject shadowMap;
	GLShaderProgram shadowShader;

	static unsigned int getResolution();

	// for rendering
	GLShaderProgram def;
	GLMesh mesh;
};

#endif // LIGHT_HPP
