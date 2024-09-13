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

#include "MainWin.hpp"

bool& Animator::Timer::videomode()
{
	static bool videomode = false;
	return videomode;
}

void Animator::setPersonHeight(float personHeight)
{
	this->personHeight = personHeight;
	QSettings().setValue("misc/disttoorigin", personHeight);
	emit personHeightChanged(personHeight);
}

Scene Animator::getCurrentScene() const
{
	auto result = Scene::getCurrentState(universe, tmm);
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

// NOLINTNEXTLINE(performance-unnecessary-value-param)
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
	emit transitionsModified();
}

void Animator::setTransition(int newid)
{
	if(newid < 0 || newid >= static_cast<int>(transitions.size()))
	{
		return;
	}
	executeTransition(transitions[newid]);
	setId(newid);
}

void Animator::update(float frameTiming, bool videomode)
{
	Timer::videomode() = videomode;
	timer.update(frameTiming);
	if(idleMode && !playCustom)
	{
		if(idleModeForward)
		{
			if(idBackup + 1 < static_cast<int>(transitions.size()))
			{
				next();
			}
			else
			{
				previous();
				idleModeForward = false;
			}
		}
		else
		{
			if(idBackup - 1 >= 0)
			{
				previous();
			}
			else
			{
				next();
				idleModeForward = true;
			}
		}
	}

	if(!timer.isValid())
	{
		return;
	}
	const float t_secs = pausedAt + timer.elapsed();

	OrbitalSystemRenderer::autoCameraTarget = false;
	Universe::setLabelsOrbitsOnly({});

	universe.setCamYaw(shiftHorizontalAngle);
	universe.setCamPitch(-shiftVerticalAngle);
	shiftHorizontalAngle = 0.0;
	shiftVerticalAngle   = 0.05;

	if(playCustom)
	{
		tmm.exposure = 0.3;
		fadeFactor   = 1.0;
		float d      = customTransition.getDuration();
		if(idleMode)
		{
			d *= 5.f;
		}
		if(!customTransition.updateUniverse(
		       universe, tmm, t_secs / d, currentScene, fadeFactor,
		       [this]() { return getCosmoShift(); },
		       [this]() { return getPlanetShift(); }))
		{
			stop();
		}
		tmm.exposure *= fadeFactor;
	}
	else if(!transitions.empty())
	{
		Transition const* currentTransition = transitions.data();
		unsigned int i(1);
		float durationSum(0.f);
		while(i < transitions.size()
		      && t_secs > currentTransition->getDuration() + durationSum)
		{
			durationSum += currentTransition->getDuration();
			currentTransition = &transitions[i];
			setId(i);
			++i;
		}
		if(t_secs <= currentTransition->getDuration() + durationSum)
		{
			tmm.exposure = 0.3;
			fadeFactor   = 1.0;
			const float t_harsh((t_secs - durationSum)
			                    / currentTransition->getDuration());
			if(currentTransition == transitions.data())
			{
				currentTransition->updateUniverse(
				    universe, tmm, t_harsh, currentTransition->getDestination(),
				    fadeFactor, [this]() { return getCosmoShift(); },
				    [this]() { return getPlanetShift(); });
			}
			else
			{
				Vector3 planetdPos, cosmodPos;
				if(debug
				   && (t_harsh * currentTransition->getDuration() < 0.03
				       || (1.0 - t_harsh) * currentTransition->getDuration()
				              < 0.03))
				{
					planetdPos = Vector3() - universe.getPlanetPosition();
					cosmodPos  = Vector3() - universe.getCosmoPosition();
				}
				currentTransition->updateUniverse(
				    universe, tmm, t_harsh, transitions[i - 2].getDestination(),
				    fadeFactor, [this]() { return getCosmoShift(); },
				    [this]() { return getPlanetShift(); });
				if(debug
				   && (t_harsh * currentTransition->getDuration() < 0.1
				       || (1.0 - t_harsh) * currentTransition->getDuration()
				              < 0.1))
				{
					planetdPos += universe.getPlanetPosition();
					cosmodPos += universe.getCosmoPosition();
					const float dt = t_secs - t_secsBAK;
					qDebug()
					    << getCurrentTransitionId()
					    << currentTransition->getName()
					    << planetdPos.length() / dt << cosmodPos.length() / dt;
				}
			}
			tmm.exposure *= fadeFactor;
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

	t_secsBAK = t_secs;
}

void Animator::removeAllTransitions()
{
	transitions.clear();
	emit transitionsModified();
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
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
		ret += timer.elapsed();
	}
	return 100.f * ret / getTotalDuration();
}

void Animator::setWholeAnimationPercentage(float percentage)
{
	const float time(getTotalDuration() * percentage / 100.f);
	stop();
	pausedAt = time;
	play();
	voiceover.setPosition(time * 1000.f);
}

void Animator::setIdleMode(bool idleMode)
{
	idleModeForward = true;
	if(QSettings().value("misc/idlemode").toBool())
	{
		this->idleMode = idleMode;
		if(!this->idleMode)
		{
			stop();
		}
	}
	else
	{
		this->idleMode = false;
	}
}

void Animator::setFirstScene()
{
	if(transitions.empty())
	{
		return;
	}
	executeTransition(transitions[0]);
}

void Animator::restart()
{
	animationsDisabled = true;
	stop();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	voiceover.setSource(mainWin.getVoiceoverUrl());
#else
	voiceover.setMedia(mainWin.getVoiceoverUrl());
#endif
	play();
	animationsDisabled = false;
}

void Animator::play()
{
	timer.restart();
	voiceover.play();
	emit resumed();
}

void Animator::pause()
{
	pausedAt += timer.elapsed();
	timer.invalidate();
	voiceover.pause();
	emit paused();
}

void Animator::stop()
{
	setId(-1);
	OrbitalSystemRenderer::autoCameraTarget = true;
	playCustom                              = false;
	pausedAt                                = 0.f;
	timer.invalidate();
	voiceover.stop();
	emit stopped();
}

QString Animator::getPythonRepresentation() const
{
	QString result
	    = "from PythonQt.QtCore import Qt, QDateTime, QDate, QTime, QTimeZone\n"
	      "from PythonQt.QtMultimedia import QSound\n"
	      "from PythonQt.libplanet import Vector3\n"
	      "from PythonQt.virup import Transition, Scene, SceneSpatialData, "
	      "SceneTemporalData, SceneUI\n"
	      "transitions = [\n";
	for(auto const& t : transitions)
	{
		result += "    ";
		result += t.getPythonRepresentation().replace('\n', "\n    ");
		result += ",\n";
	}

	result += "]\n"
	          "def initScene():\n"
	          "    Animator.removeAllTransitions()\n"
	          "    for t in transitions:\n"
	          "        Animator.appendTransition(t)\n"
	          "    Animator.restart()\n";
	return result;
}
