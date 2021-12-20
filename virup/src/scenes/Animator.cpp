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
	auto sd     = result.getSpatialData();
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

void Animator::appendTransition(Transition t)
{
	/*
	if(transitions.size() > 0)
	{
	    auto scene  = t.getDestination();
	    auto td     = scene.getTemporalData();
	    auto prevTd = transitions[transitions.size() - 1]
	                      .getDestination()
	                      .getTemporalData();
	    if(prevTd.getTimeCoeff() == td.getTimeCoeff()
	       && !td.getSimulationTime().isValid()
	       && prevTd.getSimulationTime().isValid())
	    {
	        SceneTemporalData newTd(
	            td.getTimeCoeff(),
	            prevTd.getSimulationTime().addSecs(t.getDuration() *
	td.getTimeCoeff())); scene.setTemporalData(newTd);
	        appendTransition(Transition(scene, t.getDuration(), t.getName(),
	                                    t.getCustomPythonFunction()));
	        return;
	    }
	}*/
	transitions.push_back(std::move(t));
}

void Animator::setTransition(int newid)
{
	if(newid < 0 || newid >= static_cast<int>(transitions.size()))
	{
		stop();
		return;
	}
	executeTransition(transitions[newid]);
}

void Animator::update()
{
	if(!timer.isValid())
	{
		return;
	}
	float t_secs = pausedAt + timer.elapsed() * 0.001f;

	OrbitalSystemRenderer::autoCameraTarget = false;
	tmm.exposure                            = 0.3;
	fadeFactor                              = 1.0;
	universe.setLabelsOrbitsOnly({});

	universe.setCamYaw(shiftHorizontalAngle);
	universe.setCamPitch(-shiftVerticalAngle);
	shiftHorizontalAngle = 0.0;
	shiftVerticalAngle   = 0.05;

	if(playCustom)
	{
		if(!customTransition.updateUniverse(
		       universe, t_secs / customTransition.getDuration(), currentScene,
		       fadeFactor, getCosmoShift(), getPlanetShift()))
		{
			stop();
		}
	}
	else if(!transitions.empty())
	{
		Transition const* currentTransition = &transitions[0];
		unsigned int i(1);
		float durationSum(0.f);
		while(i < transitions.size()
		      && t_secs > currentTransition->getDuration() + durationSum)
		{
			durationSum += currentTransition->getDuration();
			currentTransition = &transitions[i];
			id                = i;
			++i;
		}
		if(t_secs <= currentTransition->getDuration() + durationSum)
		{
			float t_harsh((t_secs - durationSum)
			              / currentTransition->getDuration());
			if(currentTransition == &transitions[0])
			{
				currentTransition->updateUniverse(
				    universe, t_harsh, currentTransition->getDestination(),
				    fadeFactor, getCosmoShift(), getPlanetShift());
			}
			else
			{
				currentTransition->updateUniverse(
				    universe, t_harsh, transitions[i - 2].getDestination(),
				    fadeFactor, getCosmoShift(), getPlanetShift());
			}
		}
		else
		{
			stop();
		}
	}
	else
	{
		stop();
	}

	tmm.exposure *= fadeFactor;
}

void Animator::executeTransition(Transition t)
{
	stop();
	play();
	playCustom       = true;
	customTransition = std::move(t);
	if(animationsDisabled)
	{
		currentScene = customTransition.getDestination();
	}
	else
	{
		currentScene = getCurrentScene();
	}
}

float Animator::getTotalDuration() const
{
	float ret(0.f);
	for(auto const& t : transitions)
	{
		ret += t.getDuration();
	}
	return ret;
}

float Animator::getWholeAnimationPercentage() const
{
	float ret(pausedAt);
	if(timer.isValid())
	{
		ret += timer.elapsed() * 0.001f;
	}
	return 100.f * ret / getTotalDuration();
}

void Animator::setWholeAnimationPercentage(float percentage)
{
	float time(getTotalDuration() * percentage / 100.f);
	stop();
	pausedAt = time;
	play();
}

void Animator::restart()
{
	animationsDisabled = true;
	stop();
	play();
	animationsDisabled = false;
	PythonQtHandler::evalScript(
	    "voiceover=QSound(VIRUP.getVoiceoverPath())\nvoiceover.play()");
}

void Animator::play()
{
	timer.restart();
}

void Animator::pause()
{
	pausedAt += timer.elapsed() / 1000.f;
	timer.invalidate();
	stopVoiceover();
}

void Animator::stop()
{
	id                                      = -1;
	OrbitalSystemRenderer::autoCameraTarget = true;
	playCustom                              = false;
	pausedAt                                = 0.f;
	timer.invalidate();
	stopVoiceover();
}

void Animator::stopVoiceover()
{
	PythonQtHandler::evalScript("__foo=('voiceover' in locals())");
	if(PythonQtHandler::getVariable("__foo").toBool())
	{
		PythonQtHandler::evalScript("del voiceover");
	}
}
