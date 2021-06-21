/*
    Copyright (C) 2020 Florian Cabot <florian.cabot@hotmail.fr>

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

#ifndef TEXTUREDSPHERE_HPP
#define TEXTUREDSPHERE_HPP

#include "Primitives.hpp"
#include "UniverseElement.hpp"

class TexturedSphere : public UniverseElement
{
  public:
	TexturedSphere(QJsonObject const& json);
	virtual BBox getBoundingBox() const override { return {}; };
	virtual void render(Camera const& camera,
	                    ToneMappingModel const& tmm) override;

	static QList<QPair<QString, QWidget*>>
	    getLauncherFields(QWidget* parent, QJsonObject* jsonObj);

  private:
	GLShaderProgram shader;
	GLMesh mesh;
	GLTexture tex;
};

#endif // TEXTUREDSPHERE_HPP
