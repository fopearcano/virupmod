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

#ifndef PLANETARYSYSTEMS_HPP
#define PLANETARYSYSTEMS_HPP

#include <QJsonDocument>

#include "graphics/OrbitalSystemCamera.hpp"
#include "graphics/renderers/OrbitalSystemRenderer.hpp"
#include "physics/OrbitalSystem.hpp"
#include "universe/UniverseElement.hpp"

class PlanetarySystems : public UniverseElement
{
  public:
	PlanetarySystems();
	virtual BBox getBoundingBox() const override { return {}; };
	bool renderSystem() const { return doRender; };
	QStringList getSystemsNames() const;
	OrbitalSystem const* getSystem(QString const& name)
	{
		return systems[ids.at(name)];
	};
	Vector3 getClosestSystemPosition()
	{
		return Utils::fromQt(getRelToAbsTransform()
		                     * Utils::toQt(positions[closestId]));
	};
	OrbitalSystem* getClosestSystem() { return systems[closestId]; };
	Vector3 getAbsolutePosition(QString const& systemName) const;
	virtual void update(Camera const& camera) override;
	virtual void render(Camera const& camera,
	                    ToneMappingModel const& tmm) override;
	~PlanetarySystems();

	bool useVRCamposForClosest = true;

  private:
	bool doRender          = false;
	unsigned int closestId = 0;
	std::vector<Vector3> positions;
	std::vector<OrbitalSystem*> systems;
	std::vector<QString> directories;
	std::map<QString, unsigned int> ids;

	GLShaderProgram shader;
	GLMesh mesh;

	QMatrix4x4 model;
	QVector3D campos;
};

#endif // PLANETARYSYSTEMS_HPP
