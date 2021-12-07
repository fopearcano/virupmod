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

#include "scenes/SceneTemporalData.hpp"

SceneTemporalData::SceneTemporalData(float timeCoeff)
    : timeCoeff(timeCoeff)
{
}

SceneTemporalData::SceneTemporalData(float timeCoeff, QDateTime simulationTime)
    : timeCoeff(timeCoeff)
    , simulationTime(std::move(simulationTime))
{
}

SceneTemporalData SceneTemporalData::getCurrentState(Universe const& universe)
{
	auto timeCoeff(universe.getTimeCoeff());
	auto simulationTime(universe.getSimulationTime());

	return {timeCoeff, simulationTime};
}

void SceneTemporalData::setAsUniverseState(Universe& universe) const
{
	universe.setTimeCoeff(timeCoeff);
	if(simulationTime.isValid())
	{
		universe.setSimulationTime(simulationTime);
	}
}

SceneTemporalData SceneTemporalData::interpolate(SceneTemporalData const& td0,
                                                 SceneTemporalData const& td1,
                                                 float t)
{
	auto timeCoeff
	    = Interpolation::interpolateLog(td0.timeCoeff, td1.timeCoeff, t);
	auto simulationTime = Interpolation::interpolateLinear(
	    td0.simulationTime, td1.simulationTime, t);
	return {timeCoeff, simulationTime};
}

QString SceneTemporalData::getPythonRepresentation() const
{
	QString result("SceneTemporalData(");

	result += QString::number(timeCoeff);

	if(simulationTime.isValid())
	{
		result += ", QDateTime(QDate(";
		result += QString::number(simulationTime.date().year());
		result += ", ";
		result += QString::number(simulationTime.date().month());
		result += ", ";
		result += QString::number(simulationTime.date().day());
		result += "), QTime(";
		result += QString::number(simulationTime.time().hour());
		result += ", ";
		result += QString::number(simulationTime.time().minute());
		result += ", ";
		result += QString::number(simulationTime.time().second());
		result += "), QTimeZone(";
		result += QString::number(
		    simulationTime.timeZone().offsetFromUtc(simulationTime));
		result += "))";
	}

	result += ")";
	return result;
}
