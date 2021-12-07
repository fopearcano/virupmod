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
                       QString customPythonFunction)
    : toScene(std::move(toScene))
    , duration(duration)
    , name(std::move(name))
    , customPythonFunction(std::move(customPythonFunction))
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

void Transition::play()
{
	timer.restart();
}

void Transition::pause()
{
	pausedAt += timer.elapsed() / 1000.f;
	timer.invalidate();
}

void Transition::stop()
{
	pausedAt = 0.f;
	timer.invalidate();
}

bool Transition::updateUniverse(Universe& universe, Scene const& fromScene,
                                float fadeFactor, Vector3 const& cosmoShift,
                                Vector3 const& planetShift)
{
	if(!timer.isValid())
	{
		return false;
	}
	float t_harsh = timer.elapsed() / (duration * 1000.f);
	float t(t_harsh);
	bool returnedVal = true;
	if(t > 1.0 || t < 0.0)
	{
		stop();
		t_harsh     = 1.0;
		t           = 1.0;
		returnedVal = false;
	}

	auto scene = Scene::interpolate(fromScene, toScene, t);

	auto ui = scene.getUI();
	if(fadeFactor != 0.0)
	{
		QStringList names({"Constellations", "Orbits", "PlanetsLabels"});
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
	custom(t, t_harsh);

	if(scene.getSpatialData().getBodyName() != ""
	   && universe.isPlanetarySystemLoaded())
	{
		universe.setPlanetPosition(universe.getPlanetPosition() + planetShift);
	}
	else
	{
		universe.setCosmoPosition(universe.getCosmoPosition() + cosmoShift);
	}

	return returnedVal;
}

void Transition::applyDestination(Universe& universe, float fadeFactor,
                                  Vector3 const& cosmoShift,
                                  Vector3 const& planetShift) const
{
	auto scene(toScene);
	auto ui = scene.getUI();
	if(fadeFactor != 0.0)
	{
		QStringList names({"Constellations", "Orbits", "PlanetsLabels"});
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
		universe.setPlanetPosition(universe.getPlanetPosition() + planetShift);
	}
	else
	{
		universe.setCosmoPosition(universe.getCosmoPosition() + cosmoShift);
	}
}
