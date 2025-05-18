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

#include "scenes/SceneToneMappingData.hpp"

#include "scenes/Interpolation.hpp"

SceneToneMappingData::SceneToneMappingData(float exposure, float contrast,
                                           float dynamicrange)
    : exposure(exposure)
    , contrast(contrast)
    , dynamicrange(dynamicrange)
{
}

SceneToneMappingData
    SceneToneMappingData::getCurrentState(ToneMappingModel const& tmm)
{
	return SceneToneMappingData{tmm.exposure(), tmm.contrast(),
	                            tmm.dynamicrange()};
}

void SceneToneMappingData::setAsState(ToneMappingModel& tmm) const
{
	tmm.setExposure(exposure);
	tmm.setContrast(contrast);
	tmm.setDynamicrange(dynamicrange);
}

SceneToneMappingData
    SceneToneMappingData::interpolate(SceneToneMappingData const& tmd0,
                                      SceneToneMappingData const& tmd1, float t)
{
	auto e = Interpolation::interpolateLinear(tmd0.exposure, tmd1.exposure, t);
	auto c = Interpolation::interpolateLinear(tmd0.contrast, tmd1.contrast, t);
	auto d = Interpolation::interpolateLinear(tmd0.dynamicrange,
	                                          tmd1.dynamicrange, t);
	return SceneToneMappingData{e, c, d};
}

QString SceneToneMappingData::getPythonRepresentation() const
{
	QString result("SceneToneMappingData(");

	result += QString::number(exposure) + ',';
	result += QString::number(contrast) + ',';
	result += QString::number(dynamicrange) + ')';

	return result;
}
