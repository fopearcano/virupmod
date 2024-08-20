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

#ifndef OBJECTPOOL_HPP
#define OBJECTPOOL_HPP

#include <list>

#include "memory.hpp"

/**@brief A template to create pools object, useful for reusing GPU resources
 * for the whole runtime instead of constantly allocate/deallocate them.
 */
template <typename T>
class ObjectPool
{
  public:
	ObjectPool() = default;

	ObjectPool(const ObjectPool&)            = delete;
	ObjectPool& operator=(const ObjectPool&) = delete;
	ObjectPool(ObjectPool&&)                 = delete;
	ObjectPool& operator=(ObjectPool&&)      = delete;
	virtual ~ObjectPool()                    = default;

	virtual std::unique_ptr<T> createObject() const = 0;

	std::size_t getSize() { return objects.size(); };

	std::unique_ptr<T> acquire()
	{
		if(objects.empty())
		{
			return createObject();
		}

		auto obj(std::move(objects.front()));
		objects.pop_front();
		return obj;
	}

	void release(std::unique_ptr<T> object)
	{
		objects.emplace_back(std::move(object));
	}

	void reserve(std::size_t size)
	{
		while(objects.size() < size)
		{
			objects.emplace_back(std::move(createObject()));
		}
	}

  protected:
	std::list<std::unique_ptr<T>> objects;
};

#endif // OBJECTPOOL_HPP
