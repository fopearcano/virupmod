/*
    Copyright (C) 2022 Florian Cabot <florian.cabot@hotmail.fr>

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

#ifndef SCENECAMERADATA_HPP
#define SCENECAMERADATA_HPP

#include <QQuaternion>

#include "universe/Universe.hpp"

/*! \ingroup pywrap
 *
 * To use this class in a Python script :
 * \code{.py}
 * from PythonQt.virup import SceneCameraData
 * \endcode
 */
class SceneCameraData
{
  public:
	SceneCameraData()
	    : SceneCameraData(0.05f, 0.f) {};
	SceneCameraData(SceneCameraData const& other) = default;
	SceneCameraData(SceneCameraData&& other)      = default;
	SceneCameraData(float pitch, float yaw); // rad
	SceneCameraData& operator=(SceneCameraData const& other) = default;
	float getPitch() const; // rad
	float getYaw() const;   // rad
	static SceneCameraData getCurrentState(Universe const& universe);
	void setAsUniverseState(Universe& universe) const;

	static SceneCameraData interpolate(SceneCameraData const& cd0,
	                                   SceneCameraData const& cd1, float t);

	QString getPythonRepresentation() const;

  private:
	QQuaternion q = QQuaternion(0.0, 0.0, 0.0, 0.0);
};

/* PYTHONQT */

Q_DECLARE_METATYPE(SceneCameraData)

class SceneCameraDataWrapper : public PythonQtWrapper
{
	Q_OBJECT
  public:
	virtual const char* wrappedClassName() const override
	{
		return "SceneCameraData";
	}
	virtual const char* wrappedClassPackage() const override { return "virup"; }

  public Q_SLOTS:
	SceneCameraData* new_SceneCameraData() { return new SceneCameraData; }
	SceneCameraData* new_SceneCameraData(SceneCameraData const& td)
	{
		return new SceneCameraData(td);
	}
	SceneCameraData* new_SceneCameraData(float pitch, float yaw)
	{
		return new SceneCameraData(pitch, yaw);
	}

	void delete_SceneCameraData(SceneCameraData* f) { delete f; }

	// access methods
	float getPitch(SceneCameraData* cd) { return cd->getPitch(); };
	float getYaw(SceneCameraData* cd) { return cd->getYaw(); };

	SceneCameraData
	    static_SceneCameraData_getCurrentState(Universe const& universe)
	{
		return SceneCameraData::getCurrentState(universe);
	};
	void setAsUniverseState(SceneCameraData* td, Universe& universe) const
	{
		td->setAsUniverseState(universe);
	};
	SceneCameraData
	    static_SceneCameraData_interpolate(SceneCameraData const& td0,
	                                       SceneCameraData const& td1, float t)
	{
		return SceneCameraData::interpolate(td0, td1, t);
	}
};

#endif // SCENECAMERADATA_HPP
