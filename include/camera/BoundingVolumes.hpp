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

#ifndef BOUNDINGVOLUMES_HPP
#define BOUNDINGVOLUMES_HPP

#include <QVector3D>

struct BoundingSphere
{
	BoundingSphere();
	BoundingSphere(BoundingSphere const& other);
	BoundingSphere(BoundingSphere&& moved) noexcept;
	BoundingSphere& operator=(BoundingSphere const& other);
	BoundingSphere& operator=(BoundingSphere&& moved) noexcept;

	BoundingSphere(QVector3D position, float radius);
	// don't chain these if possible for performance, multiply the matrices
	// first
	BoundingSphere transformed(QMatrix4x4 const& transform) const;
	BoundingSphere merged(BoundingSphere const& other) const;

	QVector3D const& position;
	float const& radius;

  private:
	QVector3D position_;
	float radius_ = 1.f;
};

// axis aligned bounding box
struct AABB
{
	AABB(AABB const& other);
	AABB(AABB&& moved) noexcept;
	AABB& operator=(AABB const& other);
	AABB& operator=(AABB&& moved) noexcept;
	AABB(QVector3D min, QVector3D max);
	// don't chain these if possible for a tighter result and performance,
	// multiply the matrices first
	AABB transformed(QMatrix4x4 const& transform) const;
	AABB merged(AABB const& other) const;

	QVector3D const& min;
	QVector3D const& max;

  private:
	QVector3D min_;
	QVector3D max_;
};

#endif // BOUNDINGVOLUMES_HPP
