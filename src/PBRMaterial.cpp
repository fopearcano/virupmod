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

#include "PBRMaterial.hpp"

#include "Light.hpp"

PBRMaterial::PBRMaterial(AlbedoSpec const& albedoSpec,
                         OcclusionSpec const& occlusionSpec,
                         EmissiveSpec const& emissiveSpec,
                         MetallicRoughnessSpec const& metallicRoughnessSpec,
                         NormalSpec const& normalSpec)
    : shader("PBR", computeDefines(albedoSpec, occlusionSpec, emissiveSpec,
                                   metallicRoughnessSpec, normalSpec))
{
	shader.setUniform("prefiltered", 1);
	shader.setUniform("brdfLUT", 2);
	shader.setUniform("albedo", 3);
	shader.setUniform("occlusion", 4);
	shader.setUniform("emissive", 5);
	shader.setUniform("metallicRoughness", 6);
	shader.setUniform("normalTex", 7);
	shader.setUniform("shadowmap", 2, std::array<int, 2>{8, 9}.data());

	shader.setUniform("occlusionBase", occlusionSpec.occlusion);
	shader.setUniform("occlusionStrength", occlusionSpec.strength);
	shader.setUniform("emissiveBase", emissiveSpec.emissiveBase);
	shader.setUniform("emissiveFactor", emissiveSpec.factor);
	shader.setUniform("metallicRoughnessBase",
	                  QVector2D{metallicRoughnessSpec.metallic,
	                            metallicRoughnessSpec.roughness});

	if(albedoSpec.isTexture)
	{
		albedoTex = std::make_unique<GLTexture>(albedoSpec.image);
		albedoTex->setSampler(albedoSpec.sampler);
		albedoTex->generateMipmap();
	}
	if(occlusionSpec.isTexture)
	{
		occlusionTex = std::make_unique<GLTexture>(occlusionSpec.image, false);
		occlusionTex->setSampler(occlusionSpec.sampler);
		occlusionTex->generateMipmap();
	}
	if(emissiveSpec.isTexture)
	{
		emissiveTex = std::make_unique<GLTexture>(emissiveSpec.image);
		emissiveTex->setSampler(emissiveSpec.sampler);
		emissiveTex->generateMipmap();
	}
	if(metallicRoughnessSpec.isTexture)
	{
		metallicRoughnessTex
		    = std::make_unique<GLTexture>(metallicRoughnessSpec.image, false);
		metallicRoughnessTex->setSampler(metallicRoughnessSpec.sampler);
		metallicRoughnessTex->generateMipmap();
	}
	if(!normalSpec.image.isNull())
	{
		normalTex = std::make_unique<GLTexture>(normalSpec.image, false);
		normalTex->setSampler(normalSpec.sampler);
		normalTex->generateMipmap();
	}
}

PBRMaterial::PBRMaterial(QString const& directory)
    : shader("PBR", QMap<QString, QString>{{"TEXTURED_ALBEDO", "1"},
                                           {"TEXTURED_OCCLUSION", "1"},
                                           {"TEXTURED_METALLICROUGHNESS", "1"},
                                           {"TEXTURED_NORMAL", "1"}})
    , textured(true)
{
	QString path("images/" + directory + "/");
	albedoTex
	    = std::make_unique<GLTexture>(getAbsoluteDataPath(path + "albedo.png"));
	albedoTex->generateMipmap();
	occlusionTex = std::make_unique<GLTexture>(
	    getAbsoluteDataPath(path + "ao.png"), false);
	metallicRoughnessTex = std::make_unique<GLTexture>(
	    getAbsoluteDataPath(path + "metallicRoughness.png"), false);
	metallicRoughnessTex->generateMipmap();
	normalTex = std::make_unique<GLTexture>(
	    getAbsoluteDataPath(path + "normal.png"), false);
	normalTex->generateMipmap();

	shader.setUniform("prefiltered", 1);
	shader.setUniform("brdfLUT", 2);
	shader.setUniform("albedo", 3);
	shader.setUniform("occlusion", 4);
	shader.setUniform("emissive", 5);
	shader.setUniform("metallicRoughness", 6);
	shader.setUniform("normalTex", 7);
	shader.setUniform("shadowmap", 2, std::array<int, 2>{8, 9}.data());

	shader.setUniform("occlusionStrength", 1.f);
	shader.setUniform("emissiveBase", QColor(0, 0, 0));
	shader.setUniform("emissiveFactor", QVector3D(0.f, 0.f, 0.f));
}

void PBRMaterial::update(QMatrix4x4 const& modelMatrix,
                         QVector3D const& cameraWorldPos,
                         std::vector<Light const*> const& lights)
{
	shader.setUniform("model", modelMatrix);
	shader.setUniform("campos", cameraWorldPos);

	Light::setUpShader(shader, lights, {});
}

void PBRMaterial::setUpTextures(GLTexture const& irradiancemap,
                                GLTexture const& prefiltered,
                                GLTexture const& brdfLUT,
                                std::vector<GLTexture const*> const& shadowmaps)
{
	std::vector<GLTexture const*> textures = {&irradiancemap,
	                                          &prefiltered,
	                                          &brdfLUT,
	                                          albedoTex.get(),
	                                          occlusionTex.get(),
	                                          emissiveTex.get(),
	                                          metallicRoughnessTex.get(),
	                                          normalTex.get()};
	for(auto shadowmap : shadowmaps)
	{
		textures.push_back(shadowmap);
	}
	GLHandler::useTextures(textures);
}

QMap<QString, QString> PBRMaterial::computeDefines(
    AlbedoSpec const& albedoSpec, OcclusionSpec const& occlusionSpec,
    EmissiveSpec const& emissiveSpec,
    MetallicRoughnessSpec const& metallicRoughnessSpec,
    NormalSpec const& normalSpec)
{
	QMap<QString, QString> result;
	if(albedoSpec.isTexture)
	{
		result["TEXTURED_ALBEDO"] = "1";
	}
	if(occlusionSpec.isTexture)
	{
		result["TEXTURED_OCCLUSION"] = "1";
	}
	if(emissiveSpec.isTexture)
	{
		result["TEXTURED_EMISSIVE"] = "1";
	}
	if(metallicRoughnessSpec.isTexture)
	{
		result["TEXTURED_METALLICROUGHNESS"] = "1";
	}
	if(!normalSpec.image.isNull())
	{
		result["TEXTURED_NORMAL"] = "1";
	}
	return result;
}
