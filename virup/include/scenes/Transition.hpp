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

/*! \ingroup pywrap
 *
 * To use this class in a Python script :
 * \code{.py}
 * from PythonQt.virup import Transition
 * \endcode
 */
class Transition
{
  public:
	Transition() = default;
	explicit Transition(Scene toScene, float duration = 10.f, QString name = "",
	                    QString customPythonFunction = "", float v0 = 0.f,
	                    float v1 = 0.f);
	Transition(Scene toScene, float duration, QString name,
	           std::function<void(float, float)> const& customFunction,
	           float v0 = 0.f, float v1 = 0.f);

	Scene getDestination() const { return toScene; };
	float getDuration() const { return duration; };
	QString getName() const { return name; };
	QString getCustomPythonFunction() const { return customPythonFunction; };
	void custom(float t, float t_harsh) const;

	/* Returns if the transition is running or not.
	 *
	 * Can return false if either :
	 * - the transition wasn't being played
	 * - the transition finished and is stopping by itself
	 */
	bool updateUniverse(Universe& universe, ToneMappingModel& tmm,
	                    float t_harsh, Scene const& fromScene, float fadeFactor,
	                    std::function<Vector3()> const& getCosmoShift,
	                    std::function<Vector3()> const& getPlanetShift) const;

	void applyDestination(Universe& universe, ToneMappingModel& tmm,
	                      float fadeFactor,
	                      std::function<Vector3()> const& getCosmoShift,
	                      std::function<Vector3()> const& getPlanetShift) const;

	QString getPythonRepresentation() const;

  private:
	Scene toScene;
	float duration = 10.f;
	QString name;
	QString customPythonFunction;
	std::function<void(float, float)> customFunction;
	float v0 = 0.f;
	float v1 = 0.f;

	static float smoothstep(float t, float v0 = 0.f, float v1 = 0.f);
};

/* PYTHONQT */

Q_DECLARE_METATYPE(Transition)

class TransitionWrapper : public PythonQtWrapper
{
	Q_OBJECT
  public:
	const char* wrappedClassName() const override { return "Transition"; }
	const char* wrappedClassPackage() const override { return "virup"; }

  public Q_SLOTS:
	static Transition* new_Transition() { return new Transition; }
	static Transition* new_Transition(Transition const& t)
	{
		return new Transition(t);
	}
	static Transition* new_Transition(Scene s)
	{
		return new Transition(std::move(s));
	}
	static Transition* new_Transition(Scene s, float d)
	{
		return new Transition(std::move(s), d);
	}
	static Transition* new_Transition(Scene s, float d, QString n)
	{
		return new Transition(std::move(s), d, std::move(n));
	}
	static Transition* new_Transition(Scene s, float d, QString n, QString c)
	{
		return new Transition(std::move(s), d, std::move(n), std::move(c));
	}
	static Transition* new_Transition(Scene s, float d, QString n, QString c,
	                                  float v0)
	{
		return new Transition(std::move(s), d, std::move(n), std::move(c), v0);
	}
	static Transition* new_Transition(Scene s, float d, QString n, QString c,
	                                  float v0, float v1)
	{
		return new Transition(std::move(s), d, std::move(n), std::move(c), v0,
		                      v1);
	}

	static void delete_Transition(Transition* t) { delete t; }

	// access methods

	static Scene getDestination(Transition* t) { return t->getDestination(); };
	static float getDuration(Transition* t) { return t->getDuration(); };
	static QString getName(Transition* t) { return t->getName(); };
	static QString getCustomPythonFunction(Transition* t)
	{
		return t->getCustomPythonFunction();
	};
	static void custom(Transition* t, float t_, float t_harsh)
	{
		t->custom(t_, t_harsh);
	};
};

#endif // TRANSITION_HPP
