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

#ifndef MODEL_H
#define MODEL_H

#include <vector>

#include "AssetLoader.hpp"
#include "Light.hpp"
#include "camera/BasicCamera.hpp"
#include "gl/GLHandler.hpp"

class Model
{
  public:
	explicit Model(QString const& modelName,
	               QColor const& defaultDiffuseColor = {0xff, 0x09, 0xf7});
	Model(QString const& modelName, GLShaderProgram&& shader,
	      QColor const& defaultDiffuseColor = {0xff, 0x09, 0xf7});
	GLShaderProgram const& getShader() const { return shader; };
	BoundingSphere getBoundingSphere() const
	{
		return {{}, boundingSphereRadius};
	};
	std::vector<std::pair<GLMesh const&, QMatrix4x4>> getMeshes() const;
	void generateShadowMap(QMatrix4x4 const& model,
	                       std::vector<Light const*> const& lights) const;
	// cameraPosition : in same space as geometricSpace
	void render(BasicCamera const& camera,
	            QMatrix4x4 const& model                         = QMatrix4x4(),
	            std::vector<GLTexture const*> const& shadowMaps = {},
	            GLHandler::GeometricSpace geometricSpace
	            = GLHandler::GeometricSpace::WORLD) const;
	void render(BasicCamera const& camera,
	            QMatrix4x4 const& model                 = QMatrix4x4(),
	            std::vector<Light const*> const& lights = {},
	            GLHandler::GeometricSpace geometricSpace
	            = GLHandler::GeometricSpace::WORLD) const;

  private:
	static QMap<QString, QString> setUpShaderDefines();
	GLShaderProgram shader;
	std::vector<AssetLoader::TexturedMesh> meshes;

	float boundingSphereRadius = 0.f;
};

#endif // MODEL
