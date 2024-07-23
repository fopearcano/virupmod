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

#ifndef GLQUERY_HPP
#define GLQUERY_HPP

#include "gl/glfunctions.hpp"

class GLQuery
{
  public:
	// implement those in protected if and only if they're needed for the Python
	// API
	GLQuery(GLQuery const& other)            = delete;
	GLQuery& operator=(GLQuery const& other) = delete;
	/**
	 * @brief Returns the number of allocated OpenGL queries (not the
	 * actual @ref GLQuery number of instances).
	 */
	static unsigned int getInstancesCount() { return instancesCount(); };

	// move semantics
	GLQuery(GLQuery&& other) noexcept;
	GLQuery& operator=(GLQuery&& other) noexcept;

	/**
	 * @brief Allocates a new @ref Query.
	 */
	GLQuery();
	bool wasQueried() const { return queried; };
	void queryCounter();
	uint64_t getResult() const;
	virtual ~GLQuery() { cleanUp(); };

  protected:
	/**
	 * @brief Frees the underlying OpenGL shader program.
	 */
	void cleanUp();

  private:
	GLuint glQuery = 0;
	bool queried   = false;

	bool doClean = true;
	static unsigned int& instancesCount();
};

#endif // GLQUERY_HPP
