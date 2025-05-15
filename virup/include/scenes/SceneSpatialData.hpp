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

#ifndef SCENESPATIALDATA_HPP
#define SCENESPATIALDATA_HPP

#include "Interpolation.hpp"
#include "math/Vector3.hpp"
#include "universe/Universe.hpp"

/*! \ingroup pywrap
 *
 * To use this class in a Python script :
 * \code{.py}
 * from PythonQt.virup import SceneSpatialData
 * \endcode
 */
class SceneSpatialData
{
  public:
	SceneSpatialData()                              = default;
	SceneSpatialData(SceneSpatialData const& other) = default;
	SceneSpatialData(SceneSpatialData&& other)      = default;
	// Cosmological scales
	SceneSpatialData(Universe const& universe, double invScale,
	                 Vector3 position = Vector3());
	// Planetary scales
	SceneSpatialData(Universe const& universe, QString systemName,
	                 QString bodyName, double invScale,
	                 Vector3 position = Vector3());
	SceneSpatialData& operator=(SceneSpatialData const& other) = default;
	SceneSpatialData& operator=(SceneSpatialData&& other)      = default;
	~SceneSpatialData()                                        = default;

	Universe const& getUniverse() const { return *universe; };

	Vector3 getPosition() const { return position; };
	void setPosition(Vector3 position) { this->position = position; };
	double getScale() const { return scale; };
	void setScale(double scale) { this->scale = scale; };
	QString getSystemName() const { return systemName; };
	void setSystemName(QString systemName)
	{
		this->systemName = std::move(systemName);
	};
	QString getBodyName() const { return bodyName; };
	void setBodyName(QString const& bodyName) { this->bodyName = bodyName; };

	static SceneSpatialData getCurrentState(Universe const& universe);
	void setAsUniverseState(Universe& universe) const;

	static SceneSpatialData interpolate(SceneSpatialData const& sd0,
	                                    SceneSpatialData const& sd1, float t);

	QString getPythonRepresentation() const;

	static void setForceDirectInterpolation(bool forced);

  private:
	static SceneSpatialData
	    noFrameChangeInterpolate(SceneSpatialData const& sd0,
	                             SceneSpatialData const& sd1, float t,
	                             float positionUnit = 1.f);

	Universe const* universe = nullptr;

	Vector3 position;
	double scale = 1.0;
	QString systemName;
	QString bodyName;

	static bool& directInterpolationForced();
};

/* PYTHONQT */

Q_DECLARE_METATYPE(SceneSpatialData)

class SceneSpatialDataWrapper : public PythonQtWrapper
{
	Q_OBJECT
  public:
	const char* wrappedClassName() const override { return "SceneSpatialData"; }
	const char* wrappedClassPackage() const override { return "virup"; }

  public Q_SLOTS:
	static SceneSpatialData* new_SceneSpatialData()
	{
		return new SceneSpatialData();
	}
	static SceneSpatialData* new_SceneSpatialData(SceneSpatialData const& sd)
	{
		return new SceneSpatialData(sd);
	}
	static SceneSpatialData* new_SceneSpatialData(Universe const& u, double s,
	                                              Vector3 p)
	{
		return new SceneSpatialData(u, s, p);
	}
	static SceneSpatialData* new_SceneSpatialData(Universe const& u, double s)
	{
		return new SceneSpatialData(u, s);
	}
	static SceneSpatialData* new_SceneSpatialData(Universe const& u, QString sn,
	                                              QString bn, double s,
	                                              Vector3 p)
	{
		return new SceneSpatialData(u, std::move(sn), std::move(bn), s, p);
	}
	static SceneSpatialData* new_SceneSpatialData(Universe const& u, QString sn,
	                                              QString bn, double s)
	{
		return new SceneSpatialData(u, std::move(sn), std::move(bn), s);
	}

	static void delete_SceneSpatialData(SceneSpatialData* s) { delete s; }

	// access methods
	static Vector3 getPosition(SceneSpatialData* sd)
	{
		return sd->getPosition();
	};
	static void setPosition(SceneSpatialData* sd, Vector3 position)
	{
		sd->setPosition(position);
	};
	static double getScale(SceneSpatialData* sd) { return sd->getScale(); };
	static void setScale(SceneSpatialData* sd, double scale)
	{
		sd->setScale(scale);
	};
	static QString getSystemName(SceneSpatialData* sd)
	{
		return sd->getSystemName();
	};
	static void setSystemName(SceneSpatialData* sd, QString systemName)
	{
		sd->setSystemName(std::move(systemName));
	};
	static QString getBodyName(SceneSpatialData* sd)
	{
		return sd->getBodyName();
	};
	static void setBodyName(SceneSpatialData* sd, QString const& bodyName)
	{
		sd->setBodyName(bodyName);
	};

	static SceneSpatialData
	    static_SceneSpatialData_getCurrentState(Universe const& universe)
	{
		return SceneSpatialData::getCurrentState(universe);
	};
	static void setAsUniverseState(SceneSpatialData* sd, Universe& universe)
	{
		sd->setAsUniverseState(universe);
	};

	static SceneSpatialData static_SceneSpatialData_interpolate(
	    SceneSpatialData const& sd0, SceneSpatialData const& sd1, float t)
	{
		return SceneSpatialData::interpolate(sd0, sd1, t);
	}
	static void static_SceneSpatialData_setForceDirectInterpolation(bool forced)
	{
		SceneSpatialData::setForceDirectInterpolation(forced);
	}
};

#endif // SCENESPATIALDATA_HPP
