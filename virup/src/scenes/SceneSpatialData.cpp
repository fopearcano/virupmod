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

#include "scenes/SceneSpatialData.hpp"

SceneSpatialData::SceneSpatialData(Universe const& universe, double invScale,
                                   Vector3 position)
    : universe(&universe)
    , position(position)
    , scale(1.0 / invScale)
{
}

SceneSpatialData::SceneSpatialData(Universe const& universe, QString systemName,
                                   QString bodyName, double invScale,
                                   Vector3 position)
    : universe(&universe)
    , position(position)
    , scale(1.0 / invScale)
    , systemName(std::move(systemName))
    , bodyName(std::move(bodyName))
{
}

SceneSpatialData SceneSpatialData::getCurrentState(Universe const& universe)
{
	Vector3 position;
	QString systemName, bodyName;
	if(universe.isPlanetarySystemRendered())
	{
		position   = universe.getPlanetPosition();
		systemName = universe.getPlanetarySystemName();
		bodyName   = universe.getPlanetTarget();
	}
	else
	{
		position = universe.getCosmoPosition();
	}
	auto scale(universe.getScale());

	return {universe, systemName, bodyName, 1.0 / scale, position};
}

void SceneSpatialData::setAsUniverseState(Universe& universe) const
{
	universe.setScale(scale);

	if(systemName != "" && bodyName != ""
	   && universe.isPlanetarySystemRendered())
	{
		if(universe.getPlanetarySystemName() == systemName)
		{
			universe.setPlanetTarget(bodyName);
			universe.setPlanetPosition(position);
		}
		else
		{
			universe.setCosmoPosition(
			    universe.planetSystems->getAbsolutePosition(systemName));
		}
	}
	else
	{
		universe.setCosmoPosition(position);
	}
}

SceneSpatialData SceneSpatialData::interpolate(SceneSpatialData const& sd0,
                                               SceneSpatialData const& sd1,
                                               float t)
{
	if(sd0.systemName == sd1.systemName)
	{
		// if no frame change
		if(sd0.bodyName == sd1.bodyName)
		{
			float unit(1.f);
			if(sd0.systemName == "" && sd0.bodyName == "")
			{
				unit = 3.086e+19;
			}
			return noFrameChangeInterpolate(sd0, sd1, t, unit);
		}
		// or stay within a planetary system
		if(sd0.systemName != "" && sd0.bodyName != "" && sd1.bodyName != "")
		{
			auto ancestor = sd1.universe->getClosestCommonAncestorName(
			    sd0.bodyName, sd1.bodyName);
			auto start
			    = sd1.universe->getCelestialBodyPosition(
			          sd0.bodyName, ancestor, sd0.universe->getSimulationTime())
			      + sd0.position;
			auto end
			    = sd1.universe->getCelestialBodyPosition(
			          sd1.bodyName, ancestor, sd0.universe->getSimulationTime())
			      + sd1.position;
			SceneSpatialData sd0ancestor(*sd0.universe, sd0.systemName,
			                             ancestor, 1.0 / sd0.scale, start);
			SceneSpatialData sd1ancestor(*sd1.universe, sd1.systemName,
			                             ancestor, 1.0 / sd1.scale, end);
			auto result
			    = noFrameChangeInterpolate(sd0ancestor, sd1ancestor, t, 1.f);
			// if we are still focused on a body, keep it as frame of reference
			// for precision
			if(result.position == sd0ancestor.position)
			{
				auto trueResult(sd0);
				trueResult.scale = result.scale;
				return trueResult;
			}
			if(result.position == sd1ancestor.position)
			{
				auto trueResult(sd1);
				trueResult.scale = result.scale;
				return trueResult;
			}
			return result;
		}
	}

	// if diving into a system
	if(sd0.systemName == "")
	{
		SceneSpatialData sd1Cosmo(
		    *sd1.universe, 1.0 / sd1.scale,
		    sd1.universe->planetSystems->getAbsolutePosition(sd1.systemName));

		auto result = noFrameChangeInterpolate(sd0, sd1Cosmo, t, 3.086e+19);
		if(sd0.universe->isPlanetarySystemRendered())
		{
			return {*sd1.universe, sd1.systemName, sd1.bodyName,
			        1.0 / result.scale, sd1.position};
		}
		return result;
	}
	// or zooming out of a system
	if(sd1.systemName == "")
	{
		SceneSpatialData sd0Cosmo(
		    *sd0.universe, 1.0 / sd0.scale,
		    sd0.universe->planetSystems->getAbsolutePosition(sd0.systemName));

		auto result = noFrameChangeInterpolate(sd0Cosmo, sd1, t, 3.086e+19);
		if(sd0.universe->isPlanetarySystemRendered())
		{
			return {*sd0.universe, sd0.systemName, sd0.bodyName,
			        1.0 / result.scale, sd0.position};
		}
		return result;
	}
	// or switching planetary system
	SceneSpatialData sd0Cosmo(
	    *sd0.universe, 1.0 / sd0.scale,
	    sd0.universe->planetSystems->getAbsolutePosition(sd0.systemName));
	SceneSpatialData sd1Cosmo(
	    *sd1.universe, 1.0 / sd1.scale,
	    sd1.universe->planetSystems->getAbsolutePosition(sd1.systemName));

	auto result = noFrameChangeInterpolate(sd0Cosmo, sd1Cosmo, t, 3.086e+19);
	if(sd0.universe->isPlanetarySystemRendered() && t < 0.5f)
	{
		return {*sd0.universe, sd0.systemName, sd0.bodyName, 1.0 / result.scale,
		        sd0.position};
	}
	if(sd0.universe->isPlanetarySystemRendered() && t >= 0.5f)
	{
		return {*sd1.universe, sd1.systemName, sd1.bodyName, 1.0 / result.scale,
		        sd1.position};
	}
	return result;
}

QString SceneSpatialData::getPythonRepresentation() const
{
	QString result("SceneSpatialData(Universe, ");

	if(systemName != "" || bodyName != "")
	{
		result += "'" + systemName + "', '" + bodyName + "', ";
	}

	result += QString::number(1.0 / scale);
	if(position != Vector3())
	{
		result += ", Vector3(";
		result += QString::number(position[0]);
		result += ", ";
		result += QString::number(position[1]);
		result += ", ";
		result += QString::number(position[2]);
		result += ")";
	}
	result += ')';

	return result;
}

SceneSpatialData
    SceneSpatialData::noFrameChangeInterpolate(SceneSpatialData const& sd0,
                                               SceneSpatialData const& sd1,
                                               float t, float positionUnit)
{
	auto dist = (sd0.position - sd1.position).length() * positionUnit;

	// if position change will look insignificant (less than a cm in VR), then
	// go directly ; or if scale doesn't change, don't travel more than 1.1
	// meter in VR
	if(dist * sd0.scale < 0.01 || dist * sd1.scale < 0.01
	   || (sd0.scale == sd1.scale && dist * sd0.scale <= 1.1))
	{
		// avoid change of position at low scales, but go linearly if scale
		// change is small
		float posT(sd0.scale < sd1.scale ? t * 100.f : 100.f * (t - 99.f));
		if(posT < 0.f)
		{
			posT = 0.f;
		}
		if(posT > 1.f)
		{
			posT = 1.f;
		}
		if(sd0.scale / sd1.scale < 10.0 && sd1.scale / sd0.scale < 10.0)
		{
			posT = t;
		}
		auto position = Interpolation::interpolateLinear(sd0.position,
		                                                 sd1.position, posT);

		auto scale
		    = 1.0 / Interpolation::interpolateLog(sd0.scale, sd1.scale, t);

		return {*sd1.universe, sd0.systemName, sd0.bodyName, scale, position};
	}

	// intermediate scale should be dist (for distance to be one meter in VR),
	// but if scale0 or scale1 is inferior, one of them should be the
	// intermediate step

	if(sd0.scale > 1.0 / dist && sd1.scale > 1.0 / dist)
	{
		SceneSpatialData inter0(*sd1.universe, sd0.systemName, sd0.bodyName,
		                        dist, sd0.position);
		SceneSpatialData inter1(*sd1.universe, sd0.systemName, sd0.bodyName,
		                        dist, sd1.position);
		if(t <= 0.25)
		{
			return noFrameChangeInterpolate(sd0, inter0, t * 4);
		}
		if(t <= 0.75)
		{
			return noFrameChangeInterpolate(inter0, inter1, t * 2 - 0.5);
		}
		return noFrameChangeInterpolate(inter1, sd1, t * 4 - 3);
	}

	if(sd0.scale < 1.0 / dist && sd0.scale < sd1.scale)
	{
		SceneSpatialData inter(*sd1.universe, sd0.systemName, sd0.bodyName,
		                       1.0 / sd0.scale, sd1.position);
		if(t <= 0.5)
		{
			return noFrameChangeInterpolate(sd0, inter, t * 2);
		}
		return noFrameChangeInterpolate(inter, sd1, t * 2 - 1);
	}
	SceneSpatialData inter(*sd1.universe, sd0.systemName, sd0.bodyName,
	                       1.0 / sd1.scale, sd0.position);
	if(t <= 0.5)
	{
		return noFrameChangeInterpolate(sd0, inter, t * 2);
	}
	return noFrameChangeInterpolate(inter, sd1, t * 2 - 1);
}
