/*
    Copyright (C) 2024 Florian Cabot <florian.cabot@hotmail.fr>

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

#include "camera/BoundingVolumes.hpp"

#include <QMatrix4x4>
#include <array>
#include <cfloat>

BoundingSphere::BoundingSphere()
    : BoundingSphere({}, 1.f)
{
}

BoundingSphere::BoundingSphere(BoundingSphere const& other)
    : position(position_)
    , radius(radius_)
    , position_(other.position_)
    , radius_(other.radius_)
{
}

BoundingSphere::BoundingSphere(BoundingSphere&& moved) noexcept
    : position(position_)
    , radius(radius_)
    , position_(moved.position_)
    , radius_(moved.radius_)
{
}

BoundingSphere& BoundingSphere::operator=(BoundingSphere const& other)
{
	position_ = other.position_;
	radius_   = other.radius_;

	return *this;
}

BoundingSphere& BoundingSphere::operator=(BoundingSphere&& moved) noexcept
{
	position_ = moved.position_;
	radius_   = moved.radius_;

	return *this;
}

BoundingSphere::BoundingSphere(QVector3D position, float radius)
    : position(position_)
    , radius(radius_)
    , position_(position)
    , radius_(radius)
{
}

BoundingSphere BoundingSphere::transformed(QMatrix4x4 const& transform) const
{
	QVector3D pRadius(position + QVector3D(radius, 0.f, 0.f));

	auto p  = (transform * QVector4D(position, 1.f)).toVector3D();
	pRadius = (transform * QVector4D(pRadius, 1.f)).toVector3D();

	return {p, (p - pRadius).length()};
}

// https://stackoverflow.com/a/33535438
BoundingSphere BoundingSphere::merged(BoundingSphere const& other) const
{
	QVector3D relPos(other.position - position);
	auto relDist(relPos.length());
	// if enclosed, return largest sphere
	if(relDist + radius <= other.radius)
	{
		return other;
	}
	if(relDist + other.radius <= radius)
	{
		return *this;
	}
	// else
	float R     = 0.5f * (radius + other.radius + relDist);
	QVector3D C = position + (R - radius) * relPos / relDist;
	return {C, R};
}

AABB::AABB(AABB const& other)
    : min(min_)
    , max(max_)
    , min_(other.min_)
    , max_(other.max_)
{
}

AABB::AABB(AABB&& moved) noexcept
    : min(min_)
    , max(max_)
    , min_(moved.min_)
    , max_(moved.max_)
{
}

AABB& AABB::operator=(AABB const& other)
{
	min_ = other.min_;
	max_ = other.max_;

	return *this;
}

AABB& AABB::operator=(AABB&& moved) noexcept
{
	min_ = moved.min_;
	max_ = moved.max_;

	return *this;
}

AABB::AABB(QVector3D min, QVector3D max)
    : min(min_)
    , max(max_)
    , min_(min)
    , max_(max)
{
}

AABB AABB::transformed(QMatrix4x4 const& transform) const
{
	std::array<QVector4D, 8> fullBox = {{{min.x(), min.y(), min.z(), 1.f},
	                                     {max.x(), min.y(), min.z(), 1.f},
	                                     {min.x(), max.y(), min.z(), 1.f},
	                                     {min.x(), min.y(), max.z(), 1.f},
	                                     {max.x(), max.y(), min.z(), 1.f},
	                                     {max.x(), min.y(), max.z(), 1.f},
	                                     {min.x(), max.y(), max.z(), 1.f},
	                                     {max.x(), max.y(), max.z(), 1.f}}};

	for(auto& v : fullBox)
	{
		v = transform * v;
	}

	QVector3D newMin(FLT_MAX, FLT_MAX, FLT_MAX),
	    newMax(FLT_MIN, FLT_MIN, FLT_MIN);
	for(unsigned int i(0); i < 8; ++i)
	{
		for(unsigned int j(0); j < 3; ++j)
		{
			if(fullBox.at(i)[j] < newMin[j])
			{
				newMin[j] = fullBox.at(i)[j];
			}
			if(fullBox.at(i)[j] > newMax[j])
			{
				newMax[j] = fullBox.at(i)[j];
			}
		}
	}

	return {newMin, newMax};
}

AABB AABB::merged(AABB const& other) const
{
	QVector3D newMin(min.x() < other.min.x() ? min.x() : other.min.x(),
	                 min.y() < other.min.y() ? min.y() : other.min.y(),
	                 min.z() < other.min.z() ? min.z() : other.min.z());
	QVector3D newMax(max.x() > other.max.x() ? max.x() : other.max.x(),
	                 max.y() > other.max.y() ? max.y() : other.max.y(),
	                 max.z() > other.max.z() ? max.z() : other.max.z());

	return {newMin, newMax};
}
