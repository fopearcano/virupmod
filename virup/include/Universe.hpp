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

#include <map>
#include <vector>

#include "AbstractState.hpp"
#include "CSVObjects.hpp"
#include "CosmologicalLabels.hpp"
#include "CosmologicalSimulation.hpp"
#include "Credits.hpp"
#include "PlanetarySystems.hpp"
#include "TexturedSphere.hpp"

class Universe : public QObject
{
	Q_OBJECT
  public:
	class State : public AbstractState
	{
	  public:
		State()                   = default;
		State(State const& other) = default;
		State(State&& other)      = default;
		virtual void readFromDataStream(QDataStream& stream) override
		{
			visibilities.clear();
			double foo;
			for(unsigned int i(0); i < elementsSize; ++i)
			{
				stream >> foo;
				visibilities.push_back(foo);
			}
		};
		virtual void writeInDataStream(QDataStream& stream) override
		{
			for(unsigned int i(0); i < visibilities.size(); ++i)
			{
				stream << visibilities[i];
			}
		};

		std::vector<double> visibilities;
		static unsigned int elementsSize;
	};

	void readState(AbstractState const& s)
	{
		auto const& state = dynamic_cast<State const&>(s);
		if(state.visibilities.size() != elements.size())
		{
			return;
		}
		unsigned int i(0);
		for(auto& pair : elements)
		{
			pair.second->visibility = state.visibilities[i];
			++i;
		}
	};
	void writeState(AbstractState& s) const
	{
		auto& state = dynamic_cast<State&>(s);
		state.visibilities.clear();
		for(auto& pair : elements)
		{
			state.visibilities.push_back(pair.second->visibility);
		}
	};

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

  public slots:
	Vector3 getCameraCurrentRelPosToBody(QString const& bodyName) const;
	void setAnimationTime(float t)
	{
		CosmologicalSimulation::animationTime() = t;
	}
	QStringList getUniverseElementsNames() const
	{
		QStringList result;
		for(auto pair : elements)
		{
			result << pair.first;
		}
		return result;
	}
	double getVisibility(QString const& name) const;
	void setVisibility(QString const& name, double visibility);
	Vector3 getSolarSystemPosition(QString const& name) const;
	void setSolarSystemPosition(QString const& name, Vector3 const& pos);
	void setLabelsOrbitsOnly(QStringList const& nameList);

  private:
	void updateBoundingBox(BBox const& elementBoundingBox);
	void loadClosestSystem();

	BBox boundingBox
	    = {FLT_MAX, FLT_MIN, FLT_MAX, FLT_MIN, FLT_MAX, FLT_MIN, 0.f, {}};

	std::map<QString, UniverseElement*> elements;
	QList<CosmologicalSimulation*> cosmoSims;
	QList<CSVObjects*> csvObjs;

  public:
	PlanetarySystems* planetSystems = nullptr;

  private:
	// planets
	OrbitalSystemCamera& camPlanet;
	OrbitalSystem* orbitalSystem          = nullptr;
	OrbitalSystemRenderer* systemRenderer = nullptr;
	Vector3 lastData                      = Vector3(DBL_MAX, DBL_MAX, DBL_MAX);
	Vector3 sysInWorld                    = Vector3(DBL_MAX, DBL_MAX, DBL_MAX);
	bool forceUpdateFromCosmo             = true;
	UniversalTime lastCurrentUt;
};

#endif // UNIVERSE_HPP
