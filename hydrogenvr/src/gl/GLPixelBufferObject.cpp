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

#include "gl/GLPixelBufferObject.hpp"

unsigned int& GLPixelBufferObject::instancesCount()
{
	static unsigned int instancesCount = 0;
	return instancesCount;
}

GLPixelBufferObject::GLPixelBufferObject(GLPixelBufferObject&& other) noexcept
    : buff(std::move(other.buff))
    , size(other.size)
    , bytesPerPixel(other.bytesPerPixel)
    , dataFormat(other.dataFormat)
    , mappedData(other.mappedData)
    , doClean(other.doClean)
{
	// prevent other from cleaning shader if it destroys itself
	other.doClean = false;
}

GLPixelBufferObject&
    GLPixelBufferObject::operator=(GLPixelBufferObject&& other) noexcept
{
	if(this == &other)
	{
		return *this;
	}
	cleanUp();

	buff          = std::move(other.buff);
	size          = other.size;
	bytesPerPixel = other.bytesPerPixel;
	dataFormat    = other.dataFormat;
	mappedData    = other.mappedData;
	doClean       = other.doClean;

	other.doClean = false;
	return *this;
}

GLPixelBufferObject::GLPixelBufferObject(QSize const& size,
                                         unsigned int bytesPerPixel,
                                         GLTexture::Data dataFormat)
    : buff(GL_PIXEL_UNPACK_BUFFER, size.width() * size.height() * bytesPerPixel,
           GL_STREAM_DRAW)
    , size(size)
    , bytesPerPixel(bytesPerPixel)
    , dataFormat(dataFormat)
{
	++instancesCount();
	this->dataFormat.ptr = nullptr;
}

GLTexture GLPixelBufferObject::copyContentToNewTex(bool sRGB) const
{
	unmap();

	unsigned int padding = (4 - (size.width() * bytesPerPixel) % 4) % 4;
	padding              = (padding == 0 ? 4 : (padding == 3 ? 1 : padding));
	GLHandler::glf().glPixelStorei(GL_UNPACK_ALIGNMENT, padding);
	buff.bind(); // be sure it is bound before the call to glTexImage2D

	GLTexture result{
	    GLTexture::Tex2DProperties(size.width(), size.height(), sRGB),
	    GLTexture::Sampler{}, dataFormat};
	result.setData(dataFormat);
	buff.unbind();

	return result;
}

void GLPixelBufferObject::copyContentToTex(GLTexture const& texture) const
{
	unmap();

	unsigned int padding = (4 - (size.width() * bytesPerPixel) % 4) % 4;
	padding              = (padding == 0 ? 4 : (padding == 3 ? 1 : padding));
	GLHandler::glf().glPixelStorei(GL_UNPACK_ALIGNMENT, padding);
	buff.bind();
	texture.setData(dataFormat);
	buff.unbind();
}

void GLPixelBufferObject::cleanUp()
{
	if(!doClean)
	{
		return;
	}
	--instancesCount();
	doClean = false;
}

void GLPixelBufferObject::unmap() const
{
	buff.unmap();
	mappedData = nullptr;
}
