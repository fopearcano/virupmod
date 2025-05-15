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

#ifndef INTERPOLATION_HPP
#define INTERPOLATION_HPP

#include <QDateTime>
#include <cmath>

class Interpolation
{
  public:
	Interpolation() = delete;
	template <typename T>
	static T interpolateBool(T const& x0, T const& x1, float t);
	template <typename T>
	static T interpolateLinear(T const& x0, T const& x1, float t);
	static QDateTime interpolateLinear(QDateTime const& dt0,
	                                   QDateTime const& dt1, float t)
	{
		if(!dt0.isValid() || !dt1.isValid())
		{
			return {};
		}
		const qint64 msecs
		    = static_cast<double>(t) * dt1.toMSecsSinceEpoch()
		      + (1.0 - static_cast<double>(t)) * dt0.toMSecsSinceEpoch();
		return QDateTime::fromMSecsSinceEpoch(msecs);
	}
	template <typename T>
	static T interpolateLog(T const& x0, T const& x1, float t);
};

template <typename T>
T Interpolation::interpolateBool(T const& x0, T const& x1, float t)
{
	return t < 0.5 ? x0 : x1;
}

template <typename T>
T Interpolation::interpolateLinear(T const& x0, T const& x1, float t)
{
	return x0 * (1.f - t) + x1 * t;
}

template <typename T>
T Interpolation::interpolateLog(T const& x0, T const& x1, float t)
{
	return exp(log(x0) * (1.f - t) + log(x1) * t);
}

#endif // INTERPOLATION_HPP
