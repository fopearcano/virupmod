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

#ifndef GLPIXELBUFFEROBJECT_HPP
#define GLPIXELBUFFEROBJECT_HPP

#include <QSize>
#include <span>

#include "GLBuffer.hpp"
#include "GLTexture.hpp"

class GLBuffer;
class GLHandler;

class GLPixelBufferObject
{
  public:
	// implement those in protected if and only if they're needed for the Python
	// API
	GLPixelBufferObject()                                            = delete;
	GLPixelBufferObject(GLPixelBufferObject const& other)            = delete;
	GLPixelBufferObject& operator=(GLPixelBufferObject const& other) = delete;
	/**
	 * @brief Returns the number of allocated OpenGL textures.
	 */
	static unsigned int getInstancesCount() { return instancesCount(); };

	// move semantics
	GLPixelBufferObject(GLPixelBufferObject&& other) noexcept;
	GLPixelBufferObject& operator=(GLPixelBufferObject&& other) noexcept;

	explicit GLPixelBufferObject(QSize const& size,
	                             unsigned int bytesPerPixel = 4,
	                             GLTexture::Data dataFormat = {});
	GLPixelBufferObject(unsigned int width, unsigned int height,
	                    unsigned int bytesPerPixel = 4,
	                    GLTexture::Data dataFormat = {})
	    : GLPixelBufferObject(QSize(width, height), bytesPerPixel,
	                          dataFormat){};
	QSize getSize() { return size; };
	size_t getBufferSize() const { return buff.getSize(); };
	template <typename T>
	std::span<T> getMappedData() const;
	std::unique_ptr<GLTexture> copyContentToNewTex(bool sRGB = true) const;
	void copyContentToTex(GLTexture const& texture) const;

	virtual ~GLPixelBufferObject() { cleanUp(); };

  protected:
	/**
	 * @brief Frees the underlying OpenGL buffers.
	 */
	void cleanUp();

  private:
	void unmap() const;

	GLBuffer buff;
	QSize size;
	unsigned int bytesPerPixel;
	mutable GLTexture::Data dataFormat;
	mutable void* mappedData = nullptr;

	bool doClean = true;
	static unsigned int& instancesCount();
};

template <typename T>
std::span<T> GLPixelBufferObject::getMappedData() const
{
	if(mappedData == nullptr)
	{
		mappedData = buff.map(GL_WRITE_ONLY);
	}
	return {static_cast<T*>(mappedData), getBufferSize() / sizeof(T)};
}

#endif // GLPIXELBUFFEROBJECT_HPP
