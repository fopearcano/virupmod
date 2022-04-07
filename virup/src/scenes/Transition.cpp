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

#include "scenes/Transition.hpp"

Transition::Transition(Scene toScene, float duration, QString name,
                       QString customPythonFunction, float v0, float v1)
    : toScene(std::move(toScene))
    , duration(duration)
    , name(std::move(name))
    , customPythonFunction(std::move(customPythonFunction))
    , v0(v0)
    , v1(v1)
{
}

void Transition::custom(float t, float t_harsh) const
{
	if(customPythonFunction == "")
	{
		return;
	}
	PythonQtHandler::evalScript(customPythonFunction + '(' + QString::number(t)
	                            + ',' + QString::number(t_harsh) + ')');
}

bool Transition::updateUniverse(
    Universe& universe, float t_harsh, Scene const& fromScene, float fadeFactor,
    std::function<Vector3()> const& getCosmoShift,
    std::function<Vector3()> const& getPlanetShift) const
{
	float t(smoothstep(t_harsh, v0, v1));
	bool returnedVal = true;
	if(t_harsh > 1.0 || t_harsh < 0.0)
	{
		t_harsh     = 1.0;
		t           = 1.0;
		returnedVal = false;
	}

	auto scene = Scene::interpolate(fromScene, toScene, t);

	auto ui = scene.getUI();
	if(fadeFactor != 0.0)
	{
		QStringList names({"Constellations", "Orbits", "PlanetsLabels",
		                   "Debris", "Asteroids"});
		names.append(universe.getUniverseElementsNames());
		for(auto const& n : names)
		{
			ui.setVisibility(n, ui.getVisibility(n) * fadeFactor);
		}
	}
	scene.setUI(ui);

	scene.setAsUniverseState(universe);

	universe.setAnimationTime(ui.getVisibility("AnimationTime"));

	SceneSpatialData::setForceDirectInterpolation(false);
	// apply custom function
	custom(t, t_harsh);

	if(scene.getSpatialData().getBodyName() != ""
	   && universe.isPlanetarySystemLoaded())
	{
		universe.setPlanetPosition(universe.getPlanetPosition()
		                           + getPlanetShift());
	}
	else
	{
		universe.setCosmoPosition(universe.getCosmoPosition()
		                          + getCosmoShift());
	}

	return returnedVal;
}

void Transition::applyDestination(
    Universe& universe, float fadeFactor,
    std::function<Vector3()> const& getCosmoShift,
    std::function<Vector3()> const& getPlanetShift) const
{
	auto scene(toScene);
	auto ui = scene.getUI();
	if(fadeFactor != 0.0)
	{
		QStringList names({"Constellations", "Orbits", "PlanetsLabels",
		                   "Debris", "Asteroids"});
		names.append(universe.getUniverseElementsNames());
		for(auto const& n : names)
		{
			ui.setVisibility(n, ui.getVisibility(n) * fadeFactor);
		}
	}
	scene.setUI(ui);

	scene.setAsUniverseState(universe);

	universe.setAnimationTime(ui.getVisibility("AnimationTime"));

	// apply custom function
	custom(1.f, 1.f);

	if(scene.getSpatialData().getBodyName() != ""
	   && universe.isPlanetarySystemLoaded())
	{
		universe.setPlanetPosition(universe.getPlanetPosition()
		                           + getPlanetShift());
	}
	else
	{
		universe.setCosmoPosition(universe.getCosmoPosition()
		                          + getCosmoShift());
	}
}

QString Transition::getPythonRepresentation() const
{
	QString result("Transition(");
	result += toScene.getPythonRepresentation();
	result += ", " + QString::number(duration);
	if(name != "" || customPythonFunction != "")
	{
		result += ", \"" + name + "\"";
	}
	if(customPythonFunction != "")
	{
		result += ", \"" + customPythonFunction + "\"";
	}
	result += ")";
	return result;
}

float Transition::smoothstep(float t, float v0, float v1)
{
	if(t < 0.0)
	{
		return 0.0;
	}
	if(t > 1.0)
	{
		return 1.00001;
	}
	const float t2 = t * t;
	const float t3 = t2 * t;
	const float t4 = t3 * t;
	const float t5 = t4 * t;
	return (6 * t5 - 15 * t4 + 10 * t3 + 0.5 * (v1 - v0) * t2 + v0 * t)
	       / (1.0 + 0.5 * (v1 + v0));
}
