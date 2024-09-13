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

#include "scenes/SceneCameraData.hpp"

SceneCameraData::SceneCameraData(float pitch, float yaw)
    : q(QQuaternion::fromEulerAngles(pitch * 180.f / constant::pi,
                                     yaw * 180.f / constant::pi, 0.f))
{
}

float SceneCameraData::getPitch() const
{
	float res = NAN, foo = NAN, bar = NAN;
	q.getEulerAngles(&res, &foo, &bar);
	return res * constant::pi / 180.f;
}

float SceneCameraData::getYaw() const
{
	float res = NAN, foo = NAN, bar = NAN;
	q.getEulerAngles(&foo, &res, &bar);
	return res * constant::pi / 180.f;
}

SceneCameraData SceneCameraData::getCurrentState(Universe const& universe)
{
	return {universe.getCamPitch(), universe.getCamYaw()};
}

void SceneCameraData::setAsUniverseState(Universe& universe) const
{
	float pitch = NAN, yaw = NAN, foo = NAN;
	q.getEulerAngles(&pitch, &yaw, &foo);
	universe.setCamPitch(pitch * constant::pi / 180.f);
	universe.setCamYaw(yaw * constant::pi / 180.f);
}

SceneCameraData SceneCameraData::interpolate(SceneCameraData const& cd0,
                                             SceneCameraData const& cd1,
                                             float t)
{
	SceneCameraData cd;
	cd.q = QQuaternion::slerp(cd0.q, cd1.q, t);
	return cd;
}

QString SceneCameraData::getPythonRepresentation() const
{
	float pitch = NAN, yaw = NAN, foo = NAN;
	q.getEulerAngles(&pitch, &yaw, &foo);
	QString result("SceneCameraData(");

	result += QString::number(pitch * constant::pi / 180.f) + ',';
	result += QString::number(yaw * constant::pi / 180.f) + ')';

	return result;
}
