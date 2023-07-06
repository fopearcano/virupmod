/*
    Copyright (C) 2023 Florian Cabot <florian.cabot@hotmail.fr>

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

#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <QObject>
#include <memory>

template <typename T, typename... Args>
T* qt_owned(Args... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...).release();
}

/** @brief
 * Enforces that a QObject is instanciated with a parent, this does not return a
 * smart pointer, but a regular pointer. This is more of a the Qt way of
 * ensuring memory management on heap-created objects, as make_unique would do
 * for other classes.
 *
 * The usage of make_unique<...>(...).release() is there to pass the
 * cppcoreguidelines-owning-memory test in clang-tidy.
 */
template <class C, class P, class... Args>
C* make_qt_unique(P& parent, Args&&... args)
{
	static_assert(
	    std::is_base_of<QObject, C>::value,
	    "Using make_qt_unique on a class that doesn't inherit from QObject.");
	C* ptr
	    = std::make_unique<C>(std::forward<Args>(args)..., &parent).release();
	return ptr;
}

#endif // MEMORY_HPP
