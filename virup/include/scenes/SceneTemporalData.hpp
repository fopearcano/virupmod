/*
    Copyright (C) 2021 Florian Cabot <florian.cabot@hotmail.fr>

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

#ifndef SCENETEMPORALDATA_HPP
#define SCENETEMPORALDATA_HPP

#include <QDateTime>

#include "Interpolation.hpp"
#include "universe/Universe.hpp"

class SceneTemporalData
{
  public:
	SceneTemporalData()                               = default;
	SceneTemporalData(SceneTemporalData const& other) = default;
	SceneTemporalData(SceneTemporalData&& other)      = default;
	SceneTemporalData(float timeCoeff);
	SceneTemporalData(float timeCoeff, QDateTime simulationTime);
	SceneTemporalData& operator=(SceneTemporalData const& other) = default;
	float getTimeCoeff() const { return timeCoeff; };
	QDateTime getSimulationTime() const { return simulationTime; };
	static SceneTemporalData getCurrentState(Universe const& universe);
	void setAsUniverseState(Universe& universe) const;

	static SceneTemporalData interpolate(SceneTemporalData const& td0,
	                                     SceneTemporalData const& td1, float t);

	QString getPythonRepresentation() const;

  private:
	float timeCoeff = 1.f;
	QDateTime simulationTime;
};

/* PYTHONQT */

Q_DECLARE_METATYPE(SceneTemporalData)

class SceneTemporalDataWrapper : public PythonQtWrapper
{
	Q_OBJECT
  public:
	virtual const char* wrappedClassName() const override
	{
		return "SceneTemporalData";
	}
	virtual const char* wrappedClassPackage() const override { return "virup"; }

  public Q_SLOTS:
	SceneTemporalData* new_SceneTemporalData() { return new SceneTemporalData; }
	SceneTemporalData* new_SceneTemporalData(SceneTemporalData const& td)
	{
		return new SceneTemporalData(td);
	}
	SceneTemporalData* new_SceneTemporalData(float t)
	{
		return new SceneTemporalData(t);
	}
	SceneTemporalData* new_SceneTemporalData(float t, QDateTime s)
	{
		return new SceneTemporalData(t, s);
	}

	void delete_SceneTemporalData(SceneTemporalData* f) { delete f; }

	// access methods
	float getTimeCoeff(SceneTemporalData* td) { return td->getTimeCoeff(); };
	QDateTime getSimulationTime(SceneTemporalData* td)
	{
		return td->getSimulationTime();
	};

	SceneTemporalData
	    static_SceneTemporalData_getCurrentState(Universe const& universe)
	{
		return SceneTemporalData::getCurrentState(universe);
	};
	void setAsUniverseState(SceneTemporalData* td, Universe& universe) const
	{
		td->setAsUniverseState(universe);
	};
	SceneTemporalData static_SceneTemporalData_interpolate(
	    SceneTemporalData const& td0, SceneTemporalData const& td1, float t)
	{
		return SceneTemporalData::interpolate(td0, td1, t);
	}
};

#endif // SCENETEMPORALDATA_HPP
