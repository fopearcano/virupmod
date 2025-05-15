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

#ifndef SCENETONEMAPPINGDATA_HPP
#define SCENETONEMAPPINGDATA_HPP

#include "ToneMappingModel.hpp"

/*! \ingroup pywrap
 *
 * To use this class in a Python script :
 * \code{.py}
 * from PythonQt.virup import SceneToneMappingModelData
 * \endcode
 */
class SceneToneMappingData
{
  public:
	SceneToneMappingData()
	    : SceneToneMappingData(0.3f) {};
	explicit SceneToneMappingData(float exposure, float contrast = 1.f,
	                              float dynamicrange = 10000.f);
	float getExposure() const { return exposure; };
	float getContrast() const { return contrast; };
	float getDynamicRange() const { return dynamicrange; };
	static SceneToneMappingData getCurrentState(ToneMappingModel const& tmm);
	void setAsState(ToneMappingModel& tmm) const;

	static SceneToneMappingData interpolate(SceneToneMappingData const& tmd0,
	                                        SceneToneMappingData const& tmd1,
	                                        float t);

	QString getPythonRepresentation() const;

  private:
	float exposure     = 0.3f;
	float contrast     = 1.f;
	float dynamicrange = 10000.f;
};

/* PYTHONQT */

Q_DECLARE_METATYPE(SceneToneMappingData)

class SceneToneMappingDataWrapper : public PythonQtWrapper
{
	Q_OBJECT
  public:
	const char* wrappedClassName() const override
	{
		return "SceneToneMappingData";
	}
	const char* wrappedClassPackage() const override { return "virup"; }

  public Q_SLOTS:
	static SceneToneMappingData* new_SceneToneMappingData()
	{
		return new SceneToneMappingData;
	}
	static SceneToneMappingData*
	    new_SceneToneMappingData(SceneToneMappingData const& td)
	{
		return new SceneToneMappingData(td);
	}
	static SceneToneMappingData* new_SceneToneMappingData(float exposure,
	                                                      float dynamicrange,
	                                                      float contrast)
	{
		return new SceneToneMappingData(exposure, dynamicrange, contrast);
	}
	static SceneToneMappingData* new_SceneToneMappingData(float exposure)
	{
		return new SceneToneMappingData(exposure);
	}
	static SceneToneMappingData* new_SceneToneMappingData(float exposure,
	                                                      float dynamicrange)
	{
		return new SceneToneMappingData(exposure, dynamicrange);
	}

	static void delete_SceneToneMappingData(SceneToneMappingData* f)
	{
		delete f;
	}

	// access methods
	static float getExposure(SceneToneMappingData* tmd)
	{
		return tmd->getExposure();
	};
	static float getContrast(SceneToneMappingData* tmd)
	{
		return tmd->getContrast();
	};
	static float getDynamicRange(SceneToneMappingData* tmd)
	{
		return tmd->getDynamicRange();
	};

	static SceneToneMappingData
	    static_SceneToneMappingData_getCurrentState(ToneMappingModel const& tmm)
	{
		return SceneToneMappingData::getCurrentState(tmm);
	};
	static void setAsState(SceneToneMappingData* td, ToneMappingModel& tmm)
	{
		td->setAsState(tmm);
	};
	static SceneToneMappingData static_SceneToneMappingData_interpolate(
	    SceneToneMappingData const& tmd0, SceneToneMappingData const& tmd1,
	    float t)
	{
		return SceneToneMappingData::interpolate(tmd0, tmd1, t);
	}
};

#endif // SCENETONEMAPPINGDATA_HPP
