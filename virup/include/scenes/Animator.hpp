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

#ifndef ANIMATOR_HPP
#define ANIMATOR_HPP

#include "PythonQtHandler.hpp"
#include "Transition.hpp"
#include "universe/Universe.hpp"

/** @ingroup pycall
 *
 * Callable in Python as the "Animator" object.
 */
class Animator : public QObject
{
	Q_OBJECT

	Q_PROPERTY(float personheight READ getPersonHeight WRITE setPersonHeight)
	Q_PROPERTY(float fadeFactor READ getFadeFactor WRITE setFadeFactor)
	Q_PROPERTY(float shiftHorizontalAngle READ getShiftHorizontalAngle WRITE
	               setShiftHorizontalAngle)
	Q_PROPERTY(float shiftVerticalAngle READ getShiftVerticalAngle WRITE
	               setShiftVerticalAngle)
	Q_PROPERTY(bool animationsDisabled READ areAnimationsDisabled WRITE
	               setAnimationsDisabled)
	Q_PROPERTY(bool autoIdScrolling MEMBER autoIdScrolling)
	Q_PROPERTY(Scene currentScene READ getCurrentScene)
	Q_PROPERTY(bool debug MEMBER debug)
	Q_PROPERTY(bool idlemode READ getIdleMode WRITE setIdleMode)

	class Timer
	{
	  public:
		Timer() = default;
		bool isValid() const { return el != -1.f; }
		void invalidate() { el = -1.f; };
		void update(float frameTiming)
		{
			if(isValid())
			{
				el += frameTiming;
			}
		}
		float elapsed() const { return el; };
		float restart()
		{
			auto e = el;
			el     = 0.f;
			return e;
		};

	  private:
		float el = -1.f;
	};

  public:
	Animator(Universe& universe, VRHandler const& vrHandler,
	         ToneMappingModel& tmm)
	    : universe(universe)
	    , vrHandler(vrHandler)
	    , tmm(tmm)
	{
		PythonQtHandler::addObject("Animator", this);
	};

	bool areAnimationsDisabled() const { return animationsDisabled; };
	void setAnimationsDisabled(bool animationsDisabled)
	{
		this->animationsDisabled = animationsDisabled;
	};
	float getShiftHorizontalAngle() const { return shiftHorizontalAngle; };
	void setShiftHorizontalAngle(float shiftHorizontalAngle)
	{
		this->shiftHorizontalAngle = shiftHorizontalAngle;
	};
	float getShiftVerticalAngle() const { return shiftVerticalAngle; };
	void setShiftVerticalAngle(float shiftVerticalAngle)
	{
		this->shiftVerticalAngle = shiftVerticalAngle;
	};
	float getFadeFactor() const { return fadeFactor; };
	void setFadeFactor(float fadeFactor) { this->fadeFactor = fadeFactor; };
	float getPersonHeight() const { return personHeight; };
	void setPersonHeight(float personHeight);
	int getCurrentTransitionId() const { return this->id; };
	Scene getCurrentScene() const;

	std::vector<Transition> const& getTransitions() const
	{
		return transitions;
	};

	bool debug = false;

  public slots:
	Vector3 getCosmoShift() const;
	Vector3 getPlanetShift() const;
	void toggleAnimations() { animationsDisabled = !animationsDisabled; };
	void appendTransition(Transition t);
	void setTransition(int newid);
	void update(float frameTiming);
	void removeAllTransitions();
	void executeTransition(Transition t);
	float getTotalDuration() const;
	float getWholeAnimationPercentage() const;
	void setWholeAnimationPercentage(float percentage);
	bool getIdleMode() const { return idleMode; };
	void setIdleMode(bool idleMode);

	// typical controller inputs
	void recenter() { setTransition(idBackup); };
	void home() { setTransition(0); };
	void previous() { setTransition(idBackup - 1); };
	void next() { setTransition(idBackup + 1); };

	void setFirstScene();
	void restart();
	void play(bool playVoiceover = true);
	void pause(bool pauseVoiceover = true);
	void stop(bool stopVoiceover = true);
	void startVoiceover();
	void stopVoiceover();

	QString getPythonRepresentation() const;

  signals:
	void resumed();
	void paused();
	void stopped();
	void transitionsModified();
	void personHeightChanged(float personHeight);

  private:
	Vector3 getShift(double coeff = 1.0) const;
	void setId(int id)
	{
		this->id = id;
		if(id >= 0 && id < static_cast<int>(transitions.size()))
		{
			idBackup = id;
		}
	};

	Universe& universe;
	VRHandler const& vrHandler;
	ToneMappingModel& tmm;

	int id                  = 0;
	int idBackup            = 0;
	bool animationsDisabled = false;
	float personHeight      = QSettings().value("misc/disttoorigin").toDouble();
	float shiftHorizontalAngle = 0.f;
	float shiftVerticalAngle   = 0.05f;
	float fadeFactor           = 1.f;
	bool autoIdScrolling       = true;
	Scene currentScene;

	std::vector<Transition> transitions;

	bool playCustom = false;
	Transition customTransition;

	Timer timer;
	float pausedAt = 0.f;

	float t_secsBAK = 0.f;

	bool idleMode        = false;
	bool idleModeForward = true;
};

#endif // ANIMATOR_HPP
