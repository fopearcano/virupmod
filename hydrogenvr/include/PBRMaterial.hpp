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

#ifndef PBRMATERIAL_HPP
#define PBRMATERIAL_HPP

#include "gl/GLHandler.hpp"

// wtf MSVC ; OPAQUE is defined somehow and can't be used in enum
#ifdef Q_OS_WIN
#ifdef OPAQUE
#undef OPAQUE
#endif
#endif

class Light;

class PBRMaterial
{
  public:
	struct AlbedoSpec
	{
		AlbedoSpec(QColor const& base)
		    : base(base){};
		AlbedoSpec(QImage const& image, GLTexture::Sampler const& sampler)
		    : image(image)
		    , sampler(sampler)
		    , isTexture(true){};

		const QColor base;
		const QImage image;
		const GLTexture::Sampler sampler;
		const bool isTexture = false;
	};
	struct OcclusionSpec
	{
		OcclusionSpec(float occlusion, float strength)
		    : occlusion(occlusion)
		    , strength(strength){};
		OcclusionSpec(QImage const& image, GLTexture::Sampler const& sampler,
		              float strength)
		    : image(image)
		    , sampler(sampler)
		    , strength(strength)
		    , isTexture(true){};

		const QImage image;
		const GLTexture::Sampler sampler;
		const float occlusion = 1.f;
		const float strength  = 1.f;
		const bool isTexture  = false;
	};
	struct EmissiveSpec
	{
		EmissiveSpec(QColor emissiveBase, QVector3D const& factor)
		    : emissiveBase(emissiveBase)
		    , factor(factor){};
		EmissiveSpec(QImage const& image, GLTexture::Sampler const& sampler,
		             QVector3D const& factor)
		    : image(image)
		    , sampler(sampler)
		    , factor(factor)
		    , isTexture(true){};

		const QImage image;
		const GLTexture::Sampler sampler;
		const QColor emissiveBase;
		const QVector3D factor = {0.f, 0.f, 0.f};
		const bool isTexture   = false;
	};
	struct MetallicRoughnessSpec
	{
		MetallicRoughnessSpec(float metallic, float roughness)
		    : metallic(metallic)
		    , roughness(roughness){};
		MetallicRoughnessSpec(QImage const& image,
		                      GLTexture::Sampler const& sampler)
		    : image(image)
		    , sampler(sampler)
		    , isTexture(true){};

		const float metallic  = 0.f;
		const float roughness = 1.f;
		const QImage image;
		const GLTexture::Sampler sampler;
		const bool isTexture = false;
	};
	struct NormalSpec
	{
		NormalSpec() = default;
		NormalSpec(QImage const& image, GLTexture::Sampler const& sampler)
		    : image(image)
		    , sampler(sampler){};

		const QImage image;
		const GLTexture::Sampler sampler;
	};
	enum class AlphaMode
	{
		OPAQUE,
		MASK,
		BLEND,
	};

	PBRMaterial(AlbedoSpec const& albedoSpec,
	            OcclusionSpec const& occlusionSpec,
	            EmissiveSpec const& emissiveSpec,
	            MetallicRoughnessSpec const& metallicRoughnessSpec,
	            NormalSpec const& normalSpec);
	PBRMaterial(QString const& directory);
	AlphaMode getAlphaMode() const { return alphaMode; };
	void setAlphaMode(AlphaMode alphaMode) { this->alphaMode = alphaMode; };
	float getAlphaCutoff() const { return alphaCutoff; };
	void setAlphaCutoff(float alphaCutoff);
	bool isDoubleSided() const { return doubleSided; };
	void setDoubleSided(bool doubleSided) { this->doubleSided = doubleSided; };
	void update(QMatrix4x4 const& modelMatrix, QVector3D const& cameraWorldPos,
	            std::vector<Light const*> const& lights);
	GLShaderProgram const& getShader() const { return shader; };
	void setUpTextures(GLTexture const& irradiancemap,
	                   GLTexture const& prefiltered, GLTexture const& brdfLUT,
	                   std::vector<GLTexture const*> const& shadowmaps);

  private:
	GLShaderProgram shader;

	AlphaMode alphaMode = AlphaMode::OPAQUE;
	float alphaCutoff   = -1.f;
	bool doubleSided    = false;
	bool textured       = false;
	std::unique_ptr<GLTexture> albedoTex;
	std::unique_ptr<GLTexture> occlusionTex;
	std::unique_ptr<GLTexture> emissiveTex;
	std::unique_ptr<GLTexture> metallicRoughnessTex;
	std::unique_ptr<GLTexture> normalTex;

	static QMap<QString, QString>
	    computeDefines(AlbedoSpec const& albedoSpec,
	                   OcclusionSpec const& occlusionSpec,
	                   EmissiveSpec const& emissiveSpec,
	                   MetallicRoughnessSpec const& metallicRoughnessSpec,
	                   NormalSpec const& normalSpec);
};

#endif // PBRMATERIAL_HPP
