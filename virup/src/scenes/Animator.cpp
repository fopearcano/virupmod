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

#include "scenes/Animator.hpp"

Scene Animator::getCurrentScene() const
{
	auto result = Scene::getCurrentState(universe);
	auto sd     = currentScene.getSpatialData();
	if(sd.getSystemName() == "" && sd.getBodyName() == "")
	{
		sd.setPosition(sd.getPosition() - getCosmoShift());
		result.setSpatialData(sd);
	}
	else
	{
		sd.setPosition(sd.getPosition() - getPlanetShift());
		result.setSpatialData(sd);
	}
	return result;
}

Vector3 Animator::getCosmoShift() const
{
	return getShift(3.24078e-20);
}

Vector3 Animator::getPlanetShift() const
{
	return getShift();
}

Vector3 Animator::getShift(double coeff) const
{
	auto val = personHeight * coeff / universe.getScale();
	if(!vrHandler.isEnabled())
	{
		return {cos(shiftVerticalAngle) * cos(shiftHorizontalAngle) * val,
		        cos(shiftVerticalAngle) * sin(shiftHorizontalAngle) * val,
		        sin(shiftVerticalAngle) * val};
	}
	if(vrHandler.getDriverName() != "OpenVR")
	{
		return {cos(shiftVerticalAngle) * cos(shiftHorizontalAngle) * val,
		        cos(shiftVerticalAngle) * sin(shiftHorizontalAngle) * val,
		        sin(shiftVerticalAngle) * 0.05 * val};
	}
	return {0, 0, -val};
}

void Animator::setTransition(int newid)
{
	playCustom = false;
	customTransition.stop();
	auto oldid = id;
	id         = newid;
	if(oldid >= 0 && oldid < static_cast<int>(transitions.size()))
	{
		transitions[oldid].stop();
	}
	if(id < 0 || id >= static_cast<int>(transitions.size()))
	{
		return;
	}
	transitions[id].play();
	if(animationsDisabled)
	{
		currentScene = transitions[id].getDestination();
	}
	else
	{
		if(oldid == -1)
		{
			currentScene = Scene::getCurrentState(universe);
			auto sd      = currentScene.getSpatialData();
			if(sd.getSystemName() == "" && sd.getBodyName() == "")
			{
				sd.setPosition(sd.getPosition() - getCosmoShift());
				currentScene.setSpatialData(sd);
			}
			else
			{
				sd.setPosition(sd.getPosition() - getPlanetShift());
				currentScene.setSpatialData(sd);
			}
		}
		else
		{
			currentScene = transitions[oldid].getDestination();
		}
	}
}

void Animator::update()
{
	if(!playCustom && (id < 0 || id >= static_cast<int>(transitions.size())))
	{
		return;
	}
	OrbitalSystemRenderer::autoCameraTarget = false;
	tmm.exposure                            = 0.3;
	fadeFactor                              = 1.0;
	universe.setLabelsOrbitsOnly({});

	universe.setCamYaw(shiftHorizontalAngle);
	universe.setCamPitch(-shiftVerticalAngle);
	shiftHorizontalAngle = 0.0;
	shiftVerticalAngle   = 0.05;

	int nextid = -1;
	if(!playCustom
	   && !transitions[id].updateUniverse(universe, currentScene, fadeFactor,
	                                      getCosmoShift(), getPlanetShift()))
	{
		OrbitalSystemRenderer::autoCameraTarget = true;
		nextid                                  = id + 1;
	}
	else if(playCustom
	        && !customTransition.updateUniverse(universe, currentScene,
	                                            fadeFactor, getCosmoShift(),
	                                            getPlanetShift()))
	{
		OrbitalSystemRenderer::autoCameraTarget = true;
		customTransition.stop();
		playCustom = false;
	}

	if(nextid != -1)
	{
		if(nextid < static_cast<int>(transitions.size()) && autoIdScrolling)
		{
			setTransition(nextid);
		}
		else
		{
			transitions[id].applyDestination(universe, fadeFactor,
			                                 getCosmoShift(), getPlanetShift());
		}
	}

	tmm.exposure *= fadeFactor;
}

void Animator::executeTransition(Transition t)
{
	if(id >= 0 && id < static_cast<int>(transitions.size()))
	{
		setTransition(-1);
	}
	playCustom       = true;
	customTransition = std::move(t);
	customTransition.play();
	if(animationsDisabled)
	{
		currentScene = customTransition.getDestination();
	}
	else
	{
		if(id == -1)
		{
			currentScene = Scene::getCurrentState(universe);
			auto sd      = currentScene.getSpatialData();
			if(sd.getSystemName() == "" && sd.getBodyName() == "")
			{
				sd.setPosition(sd.getPosition() - getCosmoShift());
				currentScene.setSpatialData(sd);
			}
			else
			{
				sd.setPosition(sd.getPosition() - getPlanetShift());
				currentScene.setSpatialData(sd);
			}
		}
		else
		{
			currentScene = transitions[id].getDestination();
		}
	}
}
