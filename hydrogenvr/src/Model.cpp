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

#include "Model.hpp"

Model::Model(QString const& modelName, QColor const& defaultDiffuseColor)
    : shader("model", setUpShaderDefines())
{
	auto pair = AssetLoader::loadModel(modelName, shader, defaultDiffuseColor);
	boundingSphereRadius = pair.first;
	meshes               = std::move(pair.second);

	shader.setUniform("diffuse", 0);
	shader.setUniform("specular", 1);
	shader.setUniform("ambient", 2);
	shader.setUniform("emissive", 3);
	shader.setUniform("normals", 4);
	shader.setUniform("shininess", 5);
	shader.setUniform("opacity", 6);
	shader.setUniform("lightmap", 7);
	shader.setUniform("shadowmap", 2, std::array<int, 2>{8, 9}.data());
}

Model::Model(QString const& modelName, GLShaderProgram&& shader,
             QColor const& defaultDiffuseColor)
    : shader(std::move(shader))
{
	auto pair
	    = AssetLoader::loadModel(modelName, this->shader, defaultDiffuseColor);
	boundingSphereRadius = pair.first;
	meshes               = std::move(pair.second);

	this->shader.setUniform("diffuse", 0);
	this->shader.setUniform("specular", 1);
	this->shader.setUniform("ambient", 2);
	this->shader.setUniform("emissive", 3);
	this->shader.setUniform("normals", 4);
	this->shader.setUniform("shininess", 5);
	this->shader.setUniform("opacity", 6);
	this->shader.setUniform("lightmap", 7);
	this->shader.setUniform("shadowmap", 2, std::array<int, 2>{8, 9}.data());
}

std::vector<std::pair<GLMesh const&, QMatrix4x4>> Model::getMeshes() const
{
	std::vector<std::pair<GLMesh const&, QMatrix4x4>> result;
	result.reserve(meshes.size());
	for(auto const& mesh : meshes)
	{
		result.emplace_back(mesh.mesh, mesh.transform);
	}
	return result;
}

void Model::generateShadowMap(QMatrix4x4 const& model,
                              std::vector<Light const*> const& lights) const
{
	std::vector<GLMesh const*> glMeshes;
	std::vector<QMatrix4x4> models;
	for(auto const& mesh : meshes)
	{
		glMeshes.emplace_back(&mesh.mesh);
		models.push_back(model * mesh.transform);
	}
	for(auto light : lights)
	{
		light->generateShadowMap(glMeshes, models);
	}
}

void Model::render(BasicCamera const& camera, QMatrix4x4 const& model,
                   std::vector<GLTexture const*> const& shadowMaps,
                   GLHandler::GeometricSpace geometricSpace) const
{
	shader.setUniform("campos", camera.getWorldSpacePosition());

	for(auto& mesh : meshes)
	{
		std::vector<GLTexture const*> texs(
		    {&mesh.textures.at(AssetLoader::TextureType::DIFFUSE),
		     &mesh.textures.at(AssetLoader::TextureType::SPECULAR),
		     &mesh.textures.at(AssetLoader::TextureType::AMBIENT),
		     &mesh.textures.at(AssetLoader::TextureType::EMISSIVE),
		     &mesh.textures.at(AssetLoader::TextureType::NORMALS),
		     &mesh.textures.at(AssetLoader::TextureType::SHININESS),
		     &mesh.textures.at(AssetLoader::TextureType::OPACITY),
		     &mesh.textures.at(AssetLoader::TextureType::LIGHTMAP)});
		for(auto sMap : shadowMaps)
		{
			texs.push_back(sMap);
		}
		GLHandler::useTextures(texs);
		shader.setUniform("model", model * mesh.transform);
		GLHandler::setUpRender(shader, model * mesh.transform, geometricSpace);
		mesh.mesh.render();
	}
}

void Model::render(BasicCamera const& camera, QMatrix4x4 const& model,
                   std::vector<Light const*> const& lights,
                   GLHandler::GeometricSpace geometricSpace) const
{
	Light::setUpShader(shader, lights);
	std::vector<GLTexture const*> shadowMaps;
	shadowMaps.reserve(lights.size());
	for(auto light : lights)
	{
		shadowMaps.push_back(&light->getShadowMap());
	}
	render(camera, model, shadowMaps, geometricSpace);
}

QMap<QString, QString> Model::setUpShaderDefines()
{
	QMap<QString, QString> defines;
	unsigned int smoothshadows(
	    QSettings().value("graphics/smoothshadows").toUInt());
	if(smoothshadows > 0)
	{
		defines["SMOOTHSHADOWS"] = QString::number(smoothshadows * 2 + 1);
	}
	return defines;
}
