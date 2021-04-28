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

#ifndef UNIVERSE_HPP
#define UNIVERSE_HPP

#include <vector>

#include "CSVObjects.hpp"
#include "CosmologicalSimulation.hpp"
#include "PlanetarySystems.hpp"

class Universe
{
  public:
	Universe(OrbitalSystemCamera& camPlanet);
	BBox getBoundingBox() const { return boundingBox; };
	bool isPlanetarySystemRendered() const
	{
		return planetSystems->renderSystem();
	};
	QString getPlanetTarget() const;
	void setPlanetTarget(QString const& name);
	QString
	    getClosestCommonAncestorName(QString const& celestialBodyName0,
	                                 QString const& celestialBodyName1) const;
	Vector3 getCelestialBodyPosition(QString const& bodyName,
	                                 QString const& referenceBodyName,
	                                 QDateTime const& dt,
	                                 UniversalTime const& currentUt) const;
	Vector3 interpolateCoordinates(QString const& celestialBodyName0,
	                               QString const& celestialBodyName1, float t,
	                               UniversalTime const& currentUt) const;
	void updateCosmo(Camera const& cam);
	void updatePlanetarySystem(Camera const& cam,
	                           UniversalTime const& currentUt);
	void renderCosmo(Camera const& cam,
	                 ToneMappingModel const& toneMappingModel);
	void renderPlanetarySystem();
	void renderPlanetarySystemTransparent();
	~Universe();

	QString planetarySystemName = "";

  private:
	void updateBoundingBox(BBox const& elementBoundingBox);

  public:
	void loadClosestSystem();

  private:
	BBox boundingBox
	    = {FLT_MAX, FLT_MIN, FLT_MAX, FLT_MIN, FLT_MAX, FLT_MIN, 0.f, {}};
	CosmologicalSimulation* cosmologicalSim = nullptr;
	CSVObjects* hyg                         = nullptr;
	CSVObjects* sdss                        = nullptr;

  public:
	PlanetarySystems* planetSystems = nullptr;

  private:
	// in kpc
	Vector3 solarSystemDataPos = Vector3();
	std::vector<std::pair<Vector3, LabelRenderer*>> cosmoLabels;

	// planets
	OrbitalSystemCamera& camPlanet;
	OrbitalSystem* orbitalSystem          = nullptr;
	OrbitalSystemRenderer* systemRenderer = nullptr;
	Vector3 lastData                      = Vector3(DBL_MAX, DBL_MAX, DBL_MAX);
	Vector3 sysInWorld                    = Vector3(DBL_MAX, DBL_MAX, DBL_MAX);
	bool forceUpdateFromCosmo             = true;
};

#endif // UNIVERSE_HPP
