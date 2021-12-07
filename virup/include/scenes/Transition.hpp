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

#ifndef TRANSITION_HPP
#define TRANSITION_HPP

#include "Scene.hpp"

class Transition
{
  public:
	Transition()                        = default;
	Transition(Transition const& other) = default;
	Transition(Transition&& other)      = default;
	Transition(Scene toScene, float duration = 10.f, QString name = "",
	           QString customPythonFunction = "");
	Transition& operator=(Transition const& other) = default;
	Transition& operator=(Transition&& other) = default;

	Scene getDestination() const { return toScene; };
	float getDuration() const { return duration; };
	QString getName() const { return name; };
	QString getCustomPythonFunction() const { return customPythonFunction; };
	void custom(float t, float t_harsh) const;

	void play();
	void pause();
	void stop();
	/* Returns if the transition is running or not.
	 *
	 * Can return false if either :
	 * - the transition wasn't being played
	 * - the transition finished and is stopping by itself
	 */
	bool updateUniverse(Universe& universe, Scene const& fromScene,
	                    float fadeFactor, Vector3 const& cosmoShift,
	                    Vector3 const& planetShift);

	void applyDestination(Universe& universe, float fadeFactor,
	                      Vector3 const& cosmoShift,
	                      Vector3 const& planetShift) const;

  private:
	Scene toScene;
	float duration = 10.f;
	QString name;
	QString customPythonFunction;

	QElapsedTimer timer;
	float pausedAt = 0.f;
};

/* PYTHONQT */

Q_DECLARE_METATYPE(Transition)

class TransitionWrapper : public PythonQtWrapper
{
	Q_OBJECT
  public:
	virtual const char* wrappedClassName() const override
	{
		return "Transition";
	}
	virtual const char* wrappedClassPackage() const override { return "virup"; }

  public Q_SLOTS:
	Transition* new_Transition() { return new Transition; }
	Transition* new_Transition(Transition const& t)
	{
		return new Transition(t);
	}
	Transition* new_Transition(Scene s) { return new Transition(std::move(s)); }
	Transition* new_Transition(Scene s, float d)
	{
		return new Transition(std::move(s), d);
	}
	Transition* new_Transition(Scene s, float d, QString n)
	{
		return new Transition(std::move(s), d, n);
	}
	Transition* new_Transition(Scene s, float d, QString n, QString c)
	{
		return new Transition(std::move(s), d, std::move(n), std::move(c));
	}

	void delete_Transition(Transition* t) { delete t; }

	// access methods

	Scene getDestination(Transition* t) const { return t->getDestination(); };
	float getDuration(Transition* t) const { return t->getDuration(); };
	QString getName(Transition* t) const { return t->getName(); };
	QString getCustomPythonFunction(Transition* t) const
	{
		return t->getCustomPythonFunction();
	};
	void custom(Transition* t, float t_, float t_harsh) const
	{
		t->custom(t_, t_harsh);
	};

	void play(Transition* t) { t->play(); };
	void pause(Transition* t) { t->pause(); };
	void stop(Transition* t) { t->stop(); };
	bool updateUniverse(Transition* t, Universe& universe,
	                    Scene const& fromScene, float fadeFactor,
	                    Vector3 const& cosmoShift, Vector3 const& planetShift)
	{
		return t->updateUniverse(universe, fromScene, fadeFactor, cosmoShift,
		                         planetShift);
	};
};

#endif // TRANSITION_HPP
