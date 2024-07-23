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

#ifndef VOLUME_HPP
#define VOLUME_HPP

#include "camera/BasicCamera.hpp"
#include "gl/GLHandler.hpp"
#include <QElapsedTimer>

/** Represents a ray-marchable volume data with rendering-related methods.
 *
 * The given texture must have some metadata defined :
 * - float32 minx : minimum x coordinate in model space
 * - float32 maxx : maximum x coordinate in model space
 * - float32 miny : minimum y coordinate in model space
 * - float32 maxy : maximum y coordinate in model space
 * - float32 minz : minimum z coordinate in model space
 * - float32 maxz : maximum z coordinate in model space
 */
class Volume
{
  public:
	Volume(GLTexture&& tex3D);
	Volume(GLTexture&& tex3D, GLShaderProgram&& shader);
	int getRaymarchingSamples() const { return samples; };
	void setRaymarchingSamples(int samples);
	float getAbsorptionIntensity() const { return absorptionIntensity; };
	void setAbsorptionIntensity(float intensity);
	void render(BasicCamera const& camera) const;

  private:
	int samples               = 400;
	float absorptionIntensity = 80.f;
	GLTexture texture;

	GLShaderProgram volumeShader;
	GLMesh volumeCube;

	QElapsedTimer time;
};

#endif // VOLUME_HPP
