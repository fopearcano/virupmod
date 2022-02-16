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
#include "universe/CSVObjects.hpp"
#include "universe/CosmologicalLabels.hpp"
#include "universe/CosmologicalSimulation.hpp"
#include "universe/Credits.hpp"
#include "universe/PlanetarySystems.hpp"
#include "universe/TexturedSphere.hpp"

class Universe : public QObject
{
	Q_OBJECT

	/**
	 * @brief The current time of the simulation.
	 *
	 * @accessors getSimulationTime(), setSimulationTime()
	 */
	Q_PROPERTY(
	    QDateTime simulationTime READ getSimulationTime WRITE setSimulationTime)
	/**
	 * @brief The current time coefficient of the simulation.
	 *
	 * @accessors getTimeCoeff(), setTimeCoeff()
	 */
	Q_PROPERTY(float timeCoeff READ getTimeCoeff WRITE setTimeCoeff)
	Q_PROPERTY(bool lockedRealTime READ getLockedRealTime)
	Q_PROPERTY(float tanAngleLimit READ getTanAngleLimit WRITE setTanAngleLimit)
	/**
	 * @brief The current global scale of the visualization.
	 * Ratio 1 real meter / 1 visualized meter. For example, if scale == 1/1000,
	 * each real meter you see represents one kilometer. There should be no
	 * point to set scale > 1 for astrophysics.
	 *
	 * @accessors getScale(), setScale()
	 */
	Q_PROPERTY(double scale READ getScale WRITE setScale)
	/**
	 * @brief The current camera position in the visualization, relative to the
	 * cosmological space, in kpc.
	 *
	 * @accessors getCosmoPosition(), setCosmoPosition()
	 */
	Q_PROPERTY(
	    Vector3 cosmoPosition READ getCosmoPosition WRITE setCosmoPosition)
	/**
	 * @brief Wether a planetary system is loaded or not.
	 *
	 * @accessors isPlanetarySystemLoaded()
	 */
	Q_PROPERTY(bool planetarySystemLoaded READ isPlanetarySystemLoaded)
	/**
	 * @brief Name of the currently (pre-)loaded planetary system.
	 *
	 * If set, the corresponding planetary system will be loaded.
	 *
	 * The special name "Solar System" will load whichever system that is set as
	 * the "Solar System" in the launcher, whichever its actual name is. If the
	 * name isn't "Solar System", then the system must reside in a directory
	 * named after it, within the exoplanetary systems directory.
	 */
	Q_PROPERTY(QString planetarySystemName READ getPlanetarySystemName)
	/**
	 * @brief Name of the planetary system camera target.
	 *
	 * @accessors getPlanetTarget(), setPlanetTarget()
	 */
	Q_PROPERTY(QString planetTarget READ getPlanetTarget WRITE setPlanetTarget)
	/**
	 * @brief The current camera position in the visualization, relative to the
	 * planetTarget, in meters. Undefined if !planetarySystemLoaded.
	 *
	 * @accessors getPlanetPosition(), setPlanetPosition()
	 */
	Q_PROPERTY(
	    Vector3 planetPosition READ getPlanetPosition WRITE setPlanetPosition)

	/**
	 * @brief Camera's pitch in radians.
	 *
	 * @accessors getCamPitch(), setCamPitch()
	 */
	Q_PROPERTY(float camPitch READ getCamPitch WRITE setCamPitch)
	/**
	 * @brief Camera's yaw in radians.
	 *
	 * @accessors getCamYaw(), setCamYaw()
	 */
	Q_PROPERTY(float camYaw READ getCamYaw WRITE setCamYaw)
  public:
	// SPACE

	/**
	 * @getter{scale}
	 */
	double getScale() const;
	/**
	 * @setter{scale, scale}
	 */
	void setScale(double scale);
	/**
	 * @getter{cosmoPosition}
	 */
	Vector3 getCosmoPosition() const;
	/**
	 * @setter{cosmoPosition, cosmoPosition}
	 */
	void setCosmoPosition(Vector3 cosmoPosition);
	/**
	 * @getter{planetarySystemLoaded}
	 */
	bool isPlanetarySystemLoaded() const
	{
		return isPlanetarySystemRendered();
	};
	/**
	 * @getter{planetarySystemName}
	 */
	QString getPlanetarySystemName() const
	{
		return isPlanetarySystemLoaded()
		           ? planetSystems->getClosestSystem()->getName().c_str()
		           : "";
	}
	/**
	 * @getter{planetTarget}
	 */
	QString getPlanetTarget() const;
	/**
	 * @setter{planetTarget, planetTarget}
	 */
	void setPlanetTarget(QString const& name);
	/**
	 * @getter{planetPosition}
	 */
	Vector3 getPlanetPosition() const;
	/**
	 * @setter{planetPosition, planetPosition}
	 */
	void setPlanetPosition(Vector3 planetPosition);

	// TIME

	/**
	 * @getter{simulationTime}
	 */
	QDateTime getSimulationTime() const;
	/**
	 * @setter{simulationTime, simulationTime}
	 */
	void setSimulationTime(QDateTime const& simulationTime);
	/**
	 * @getter{timeCoeff}
	 */
	float getTimeCoeff() const { return clock.getTimeCoeff(); };
	/**
	 * @setter{timeCoeff, timeCoeff}
	 */
	void setTimeCoeff(float timeCoeff) { clock.setTimeCoeff(timeCoeff); };
	/**
	 * @getter{lockedRealTime}
	 */
	bool getLockedRealTime() const { return clock.getLockedRealTime(); };

	/**
	 * @getter{tanAngleLimit}
	 */
	float getTanAngleLimit() const
	{
		return OctreeLOD::getCurrentTanAngleLimit();
	};
	/**
	 * @setter{tanAngleLimit, tanAngleLimit}
	 */
	void setTanAngleLimit(float tanAngleLimit)
	{
		OctreeLOD::setCurrentTanAngleLimit(tanAngleLimit);
	};
	//
	// CAMERA ORIENTATION

	/**
	 * @getter{camPitch}
	 */
	float getCamPitch() const { return camCosmo.pitch; }
	/**
	 * @setter{camPitch, camPitch}
	 */
	void setCamPitch(float pitch);
	/**
	 * @getter{camYaw}
	 */
	float getCamYaw() const { return camCosmo.yaw; }
	/**
	 * @setter{camYaw, camYaw}
	 */
	void setCamYaw(float yaw);

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
			for(unsigned int i(0); i < elementsSize + 2; ++i)
			{
				stream >> foo;
				visibilities.push_back(foo);
			}
			double dut;
			stream >> dut;
			ut = dut;
		};
		virtual void writeInDataStream(QDataStream& stream) override
		{
			for(unsigned int i(0); i < visibilities.size(); ++i)
			{
				stream << visibilities[i];
			}
			double dut(ut);
			stream << dut;
		};

		std::vector<double> visibilities;
		static unsigned int elementsSize;
		UniversalTime ut;
	};

	void readState(AbstractState const& s)
	{
		auto const& state = dynamic_cast<State const&>(s);
		if(state.visibilities.size() != elements.size() + 2)
		{
			return;
		}
		unsigned int i(0);
		for(auto& pair : elements)
		{
			pair.second->setVisibility(state.visibilities[i]);
			++i;
		}
		setVisibility("Constellations", state.visibilities[elements.size()]);
		setVisibility("Debris", state.visibilities[elements.size()]);
		clock.setCurrentUt(state.ut);
	};
	void writeState(AbstractState& s) const
	{
		auto& state = dynamic_cast<State&>(s);
		state.visibilities.clear();
		for(auto& pair : elements)
		{
			state.visibilities.push_back(pair.second->getVisibility());
		}
		state.visibilities.push_back(getVisibility("Constellations"));
		state.visibilities.push_back(getVisibility("Debris"));
		state.ut = clock.getCurrentUt();
	};

	Universe(Camera& camCosmo, OrbitalSystemCamera& camPlanet);
	BBox getBoundingBox() const { return boundingBox; };
	UniverseElement const* getElement(QString const& name) const
	{
		return elements.at(name);
	};
	bool isPlanetarySystemRendered() const
	{
		return planetSystems->renderSystem();
	};
	SimulationTime const& getClock() const { return clock; };
	void updateCosmo();
	void updatePlanetarySystem();
	void updateClock(bool videomode, float frameTiming);
	void renderCosmo(ToneMappingModel const& toneMappingModel);
	void renderPlanetarySystem();
	void renderPlanetarySystemTransparent();
	~Universe();

  public slots:
	/**
	 * @brief Returns closest common ancestor between two planetary bodies.
	 */
	QString
	    getClosestCommonAncestorName(QString const& celestialBodyName0,
	                                 QString const& celestialBodyName1) const;
	/**
	 * @brief Returns celestial body position relative to another at a given
	 * date/time.
	 */
	Vector3 getCelestialBodyPosition(QString const& bodyName,
	                                 QString const& referenceBodyName,
	                                 QDateTime const& dt
	                                 = QDateTime::currentDateTimeUtc()) const;
	/**
	 * @brief Interpolates celestial bodies coordinates relative to their
	 * closest common ancestor.
	 */
	Vector3 interpolateCoordinates(QString const& celestialBodyName0,
	                               QString const& celestialBodyName1,
	                               float t) const;
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
	void unlockTanAngleLimit() const
	{
		OctreeLOD::unsetCurrentTanAngleLimit();
	};
	void dumpOctreesStates();

  signals:
	// if setting visibility of something that is not a UniverseElement
	// namely : Constellations, Orbits and PlanetLabels
	void nonElementVisibilityChanged(QString const& name, float visibility);

  private:
	void updateBoundingBox(BBox const& elementBoundingBox);
	void loadClosestSystem();

	BBox boundingBox
	    = {FLT_MAX, FLT_MIN, FLT_MAX, FLT_MIN, FLT_MAX, FLT_MIN, 0.f, {}};

	std::map<QString, UniverseElement*> elements;
	std::map<UniverseElement*, QString> elementsRev;
	QList<CosmologicalSimulation*> cosmoSims;
	QList<CSVObjects*> csvObjs;

  public:
	PlanetarySystems* planetSystems = nullptr;

	// 1 m = 3.24078e-20 kpc
	const double mtokpc = 3.24078e-20;

  private:
	Camera& camCosmo;

	/* PLANET SYSTEMS */

	// planets
	OrbitalSystemCamera& camPlanet;
	OrbitalSystem* orbitalSystem          = nullptr;
	OrbitalSystemRenderer* systemRenderer = nullptr;
	Vector3 lastData                      = Vector3(DBL_MAX, DBL_MAX, DBL_MAX);
	Vector3 sysInWorld                    = Vector3(DBL_MAX, DBL_MAX, DBL_MAX);
	bool forceUpdateFromCosmo             = true;
	UniversalTime lastCurrentUt;
	SimulationTime clock = SimulationTime(
	    QSettings().value("simulation/starttime").value<QDateTime>());
};

#endif // UNIVERSE_HPP
