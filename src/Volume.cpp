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

#include "Volume.hpp"

#include "Primitives.hpp"
#include "camera/BasicCamera.hpp"

Volume::Volume(GLTexture&& tex3D)
    : Volume(std::move(tex3D), GLShaderProgram("volume"))
{
	time.start();
}

Volume::Volume(GLTexture&& tex3D, GLShaderProgram&& shader)
    : texture(std::move(tex3D))
    , volumeShader(std::move(shader))
{
	if(texture.getType() != GLTexture::Type::TEX3D)
	{
		qCritical() << "Provided texture is not a 3D texture. OpenGL id :"
		            << texture.getGLTexture()
		            << " Type :" << static_cast<int>(texture.getType());
	}

	auto const& metad(texture.getMetadata());
	for(auto const& key :
	    QStringList({"minx", "maxx", "miny", "maxy", "minz", "maxz"}))
	{
		if(metad.find(key) == metad.end()
		   || QString(metad[key].typeName()) != "float")
		{
			qCritical() << "Missing metadata float32 '" << key
			            << "' in volume texture.";
		}
	}

	texture.setSampler({GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_BORDER});

	QVector3D minbbox(metad["minx"].toFloat(), metad["miny"].toFloat(),
	                  metad["minz"].toFloat());
	QVector3D maxbbox(metad["maxx"].toFloat(), metad["maxy"].toFloat(),
	                  metad["maxz"].toFloat());

	volumeShader.setUniform("minbbox", minbbox);
	volumeShader.setUniform("maxbbox", maxbbox);
	volumeShader.setUniform("samples", samples);
	volumeShader.setUniform("densityTex", 0);

	Primitives::setAsUnitCube(volumeCube, volumeShader);
}

void Volume::setRaymarchingSamples(int samples)
{
	this->samples = samples;
	volumeShader.setUniform("samples", samples);
}

void Volume::setAbsorptionIntensity(float intensity)
{
	absorptionIntensity = intensity;
	volumeShader.setUniform("absorptionIntensity", intensity);
}

void Volume::render(BasicCamera const& camera) const
{
	volumeShader.setUniform("campos", camera.getWorldSpacePosition());
	volumeShader.setUniform("time", time.elapsed() * 1.f);

	// GLHandler::glf().glDisable(GL_DEPTH_TEST);
	GLHandler::useTextures({&texture});
	GLHandler::beginTransparent(GL_ONE, GL_ONE);
	GLHandler::setBackfaceCulling(true, GL_FRONT);
	GLHandler::setUpRender(volumeShader);
	volumeCube.render(PrimitiveType::TRIANGLE_STRIP);
	GLHandler::setBackfaceCulling(true);
	GLHandler::endTransparent();
	// GLHandler::glf().glEnable(GL_DEPTH_TEST);
}
