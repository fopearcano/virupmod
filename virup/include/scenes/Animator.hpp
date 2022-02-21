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

class Animator : public QObject
{
	Q_OBJECT

	Q_PROPERTY(float fadeFactor READ getFadeFactor WRITE setFadeFactor)
	Q_PROPERTY(float shiftHorizontalAngle READ getShiftHorizontalAngle WRITE
	               setShiftHorizontalAngle)
	Q_PROPERTY(float shiftVerticalAngle READ getShiftVerticalAngle WRITE
	               setShiftVerticalAngle)
	Q_PROPERTY(bool animationsDisabled READ areAnimationsDisabled WRITE
	               setAnimationsDisabled)
	Q_PROPERTY(bool autoIdScrolling MEMBER autoIdScrolling)
	Q_PROPERTY(Scene currentScene READ getCurrentScene)
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
	void setShiftHorizontalAngle(int shiftHorizontalAngle)
	{
		this->shiftHorizontalAngle = shiftHorizontalAngle;
	};
	float getShiftVerticalAngle() const { return shiftVerticalAngle; };
	void setShiftVerticalAngle(int shiftVerticalAngle)
	{
		this->shiftVerticalAngle = shiftVerticalAngle;
	};
	float getFadeFactor() const { return fadeFactor; };
	void setFadeFactor(float fadeFactor) { this->fadeFactor = fadeFactor; };
	int getCurrentTransitionId() const { return this->id; };
	Scene getCurrentScene() const;

	std::vector<Transition> const& getTransitions() const
	{
		return transitions;
	};

  public slots:
	Vector3 getCosmoShift() const;
	Vector3 getPlanetShift() const;
	void toggleAnimations() { animationsDisabled = !animationsDisabled; };
	void appendTransition(Transition t);
	void setTransition(int newid);
	void update();
	void removeAllTransitions();
	void executeTransition(Transition t);
	float getTotalDuration() const;
	float getWholeAnimationPercentage() const;
	void setWholeAnimationPercentage(float percentage);

	void setFirstScene();
	void restart();
	void play();
	void pause();
	void stop();
	void stopVoiceover();

  signals:
	void resumed();
	void paused();
	void stopped();
	void transitionsModified();

  private:
	Vector3 getShift(double coeff = 1.0) const;

	Universe& universe;
	VRHandler const& vrHandler;
	ToneMappingModel& tmm;

	int id                     = 0;
	bool animationsDisabled    = false;
	float personHeight         = 1.5f;
	float shiftHorizontalAngle = 0.f;
	float shiftVerticalAngle   = 0.05f;
	float fadeFactor           = 1.f;
	bool autoIdScrolling       = true;
	Scene currentScene;

	std::vector<Transition> transitions;

	bool playCustom = false;
	Transition customTransition;

	QElapsedTimer timer;
	float pausedAt = 0.f;
};

#endif // ANIMATOR_HPP
