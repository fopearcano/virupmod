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

#include "gl/GLQuery.hpp"

#include "gl/GLHandler.hpp"

unsigned int& GLQuery::instancesCount()
{
	static unsigned int instancesCount = 0;
	return instancesCount;
}

GLQuery::GLQuery(GLQuery&& other) noexcept
    : glQuery(other.glQuery)
    , queried(other.queried)
    , doClean(other.doClean)
{
	// prevent other from cleaning shader if it destroys itself
	other.doClean = false;
}

GLQuery& GLQuery::operator=(GLQuery&& other) noexcept
{
	if(this == &other)
	{
		return *this;
	}
	cleanUp();

	glQuery = other.glQuery;
	queried = other.queried;
	doClean = other.doClean;

	other.doClean = false;
	return *this;
}

GLQuery::GLQuery()
{
	++instancesCount();
	GLHandler::glf().glGenQueries(1, &glQuery);
}

void GLQuery::queryCounter()
{
	queried = true;
	GLHandler::glf().glQueryCounter(glQuery, GL_TIMESTAMP);
}

uint64_t GLQuery::getResult() const
{
	if(!queried)
	{
		throw(std::logic_error(
		    "GLQuery::getResult() : queryCounter() was never called."));
	}
	GLint stopTimerAvailable = 0;
	while(stopTimerAvailable == 0)
	{
		GLHandler::glf().glGetQueryObjectiv(glQuery, GL_QUERY_RESULT_AVAILABLE,
		                                    &stopTimerAvailable);
	}

	GLuint64 time = 0;
	GLHandler::glf().glGetQueryObjectui64v(glQuery, GL_QUERY_RESULT, &time);
	return time;
}

void GLQuery::cleanUp()
{
	if(!doClean)
	{
		return;
	}
	--instancesCount();
	GLHandler::glf().glDeleteQueries(1, &glQuery);
	doClean = false;
}
