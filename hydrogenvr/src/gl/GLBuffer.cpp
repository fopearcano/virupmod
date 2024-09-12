/*
    Copyright (C) 2020 Florian Cabot <florian.cabot@hotmail.fr>

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

#include "gl/GLHandler.hpp"

#include "gl/GLBuffer.hpp"

unsigned int& GLBuffer::instancesCount()
{
	static unsigned int instancesCount = 0;
	return instancesCount;
}

GLBuffer::GLBuffer(GLBuffer&& other) noexcept
    : id(other.id)
    , currentTarget(other.currentTarget)
    , size(other.size)
    , doClean(other.doClean)
{
	// prevent other from cleaning shader if it destroys itself
	other.doClean = false;
}

GLBuffer& GLBuffer::operator=(GLBuffer&& other) noexcept
{
	if(this == &other)
	{
		return *this;
	}
	cleanUp();

	id            = other.id;
	currentTarget = other.currentTarget;
	size          = other.size;
	doClean       = other.doClean;

	other.doClean = false;
	return *this;
}

GLBuffer::GLBuffer(GLenum target, size_t size, GLenum usage)
    : currentTarget(target)
{
	++instancesCount();

	GLHandler::glf().glGenBuffers(1, &id);
	if(size != 0)
	{
		setData(static_cast<char*>(nullptr), size, usage);
	}
}

void GLBuffer::bind() const
{
	GLHandler::glf().glBindBuffer(currentTarget, id);
}

void GLBuffer::bind(GLenum target)
{
	currentTarget = target;
	bind();
}

void GLBuffer::unbind() const
{
	GLHandler::glf().glBindBuffer(currentTarget, 0);
}

void GLBuffer::bindBase(unsigned int index) const
{
	GLHandler::glf().glBindBufferBase(currentTarget, index, id);
}

void* GLBuffer::map(GLenum access) const
{
	bind();
	auto* res = GLHandler::glf().glMapBuffer(currentTarget, access);
	unbind();
	return res;
}

void* GLBuffer::mapRange(size_t offset, size_t subSize, GLenum access) const
{
	bind();
	auto* res = GLHandler::glf().glMapBufferRange(currentTarget, offset,
	                                              subSize, access);
	unbind();
	return res;
}

void GLBuffer::unmap() const
{
	bind();
	GLHandler::glf().glUnmapBuffer(currentTarget);
	unbind();
}

void GLBuffer::cleanUp()
{
	if(!doClean)
	{
		return;
	}
	unbind();
	--instancesCount();
	GLHandler::glf().glDeleteBuffers(1, &id);
	doClean = false;
}

void GLBuffer::glBufferData(GLenum target, size_t size, void const* data,
                            GLenum usage) const
{
	if(this->size != 0 && this->size != size)
	{
		qWarning()
		    << "Resizing GLBuffer is deprecated and soon won't be possible.";
	}
	bind();
	GLHandler::glf().glBufferData(target, size, data, usage);
	unbind();
}

void GLBuffer::glBufferSubData(GLenum target, size_t offset, size_t size,
                               void const* data) const
{
	bind();
	GLHandler::glf().glBufferSubData(target, offset, size, data);
	unbind();
}
