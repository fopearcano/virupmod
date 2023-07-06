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

/**@brief A template to create pools object, useful for reusing GPU resources
 * for the whole runtime instead of constantly allocate/deallocate them.
 */
template <typename T>
class ObjectPool
{
  protected:
	std::list<T*> objects;

  public:
	ObjectPool() = default;

	ObjectPool(const ObjectPool&)            = delete;
	ObjectPool& operator=(const ObjectPool&) = delete;
	ObjectPool(ObjectPool&&)                 = delete;
	ObjectPool& operator=(ObjectPool&&)      = delete;

	virtual ~ObjectPool()
	{
		while(!objects.empty())
		{
			delete objects.back();
			objects.pop_back();
		}
	}

	virtual T* createObject() const = 0;

	std::size_t getSize() { return objects.size(); };

	T* acquire()
	{
		if(objects.empty())
		{
			objects.push_back(createObject());
		}

		T* obj = objects.back();
		objects.pop_back();
		return obj;
	}

	void release(T* object) { objects.push_back(object); }

	void reserve(std::size_t size)
	{
		while(objects.size() < size)
		{
			objects.push_back(createObject());
		}
	}
};

#endif // OBJECTPOOL_HPP
