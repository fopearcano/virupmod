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

#include "gl/GLMesh.hpp"

unsigned int& GLMesh::instancesCount()
{
	static unsigned int instancesCount = 0;
	return instancesCount;
}

GLMesh::GLMesh(GLMesh&& other) noexcept
    : vao(other.vao)
    , vbo(std::move(other.vbo))
    , ebo(std::move(other.ebo))
    , vertexSize(other.vertexSize)
    , doClean(other.doClean)
{
	// prevent other from cleaning shader if it destroys itself
	other.doClean = false;
}

GLMesh& GLMesh::operator=(GLMesh&& other) noexcept
{
	if(this == &other)
	{
		return *this;
	}
	cleanUp();

	vao        = other.vao;
	vbo        = std::move(other.vbo);
	ebo        = std::move(other.ebo);
	vertexSize = other.vertexSize;
	doClean    = other.doClean;

	other.doClean = false;
	return *this;
}

GLMesh::GLMesh()
    : vbo(GL_ARRAY_BUFFER)
    , ebo(GL_ELEMENT_ARRAY_BUFFER)
{
	++instancesCount();
	GLHandler::glf().glGenVertexArrays(1, &vao);
}

void GLMesh::cleanUp()
{
	if(!doClean)
	{
		return;
	}
	--instancesCount();
	GLHandler::glf().glDeleteVertexArrays(1, &vao);
	doClean = false;
}

void GLMesh::setVertexShaderMapping(GLShaderProgram const& shaderProgram,
                                    std::vector<VertexAttrib> const& mapping)
{
	GLHandler::glf().glBindVertexArray(vao);

	vertexSize = 0;
	for(auto const& map : mapping)
	{
		// map position
		const GLint posAttrib = shaderProgram.getAttribLocationFromName(
		    map.name.toStdString().c_str());
		if(posAttrib != -1)
		{
			GLHandler::glf().glEnableVertexAttribArray(posAttrib);
			vbo.bind(); // binds the vbo to the vao attrib pointer
			GLHandler::glf().glVertexAttribPointer(
			    posAttrib, map.size, GL_FLOAT, GL_FALSE,
			    map.stride * sizeof(float),
			    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			    reinterpret_cast<void*>(map.offset * sizeof(float)));
		}
		vertexSize += map.size * sizeof(float);
	}

	ebo.bind();
	GLHandler::glf().glBindVertexArray(0);
}

void GLMesh::setVertexShaderMapping(
    GLShaderProgram const& shaderProgram,
    std::vector<QPair<const char*, unsigned int>> const& mapping)
{
	std::vector<VertexAttrib> finalMapping;

	size_t offset = 0, stride = 0;
	for(auto map : mapping)
	{
		stride += map.second;
	}
	for(auto map : mapping)
	{
		finalMapping.emplace_back(map.first, map.second, stride, offset);
		offset += map.second;
	}

	setVertexShaderMapping(shaderProgram, finalMapping);
}

void GLMesh::setVertexShaderMapping(
    GLShaderProgram const& shaderProgram, QStringList const& mappingNames,
    std::vector<unsigned int> const& mappingSizes)
{
	std::vector<QPair<const char*, unsigned int>> mapping;
	for(unsigned int i(0); i < mappingSizes.size(); ++i)
	{
		mapping.emplace_back(mappingNames[i].toLatin1().constData(),
		                     mappingSizes[i]);
	}
	setVertexShaderMapping(shaderProgram, mapping);
}

void GLMesh::setVertices(float const* vertices, size_t vertSize)
{
	vbo.setData(vertices, vertSize);
}

void GLMesh::setVertices(float const* vertices, size_t vertSize,
                         unsigned int const* elements, size_t elemSize)
{
	vbo.setData(vertices, vertSize);
	ebo.setData(elements, elemSize);
}

void GLMesh::setVertices(std::vector<float> const& vertices)
{
	setVertices(vertices.data(), vertices.size());
}

void GLMesh::setVertices(std::vector<float> const& vertices,
                         std::vector<unsigned int> const& elements)
{
	setVertices(vertices.data(), vertices.size(), elements.data(),
	            elements.size());
}

void GLMesh::drawArrays(unsigned int first, size_t count,
                        PrimitiveType primitiveType) const
{
	if(vertexSize == 0)
	{
		return;
	}
	if(primitiveType == PrimitiveType::AUTO)
	{
		primitiveType = (ebo.getSize() == 0) ? PrimitiveType::POINTS
		                                     : PrimitiveType::TRIANGLES;
	}

	GLHandler::glf().glBindVertexArray(vao);
	GLHandler::glf().glDrawArrays(static_cast<GLenum>(primitiveType), first,
	                              count);
	GLHandler::glf().glBindVertexArray(0);
}

void GLMesh::render() const
{
	render(primitiveType);
}

void GLMesh::render(PrimitiveType primitiveType) const
{
	if(vertexSize == 0)
	{
		return;
	}
	if(primitiveType == PrimitiveType::AUTO)
	{
		primitiveType = (ebo.getSize() == 0) ? PrimitiveType::POINTS
		                                     : PrimitiveType::TRIANGLES;
	}

	GLHandler::glf().glBindVertexArray(vao);
	if(ebo.getSize() == 0)
	{
		GLHandler::glf().glDrawArrays(static_cast<GLenum>(primitiveType), 0,
		                              vbo.getSize() / vertexSize);
	}
	else
	{
		GLHandler::glf().glDrawElements(static_cast<GLenum>(primitiveType),
		                                ebo.getSize() / sizeof(unsigned int),
		                                GL_UNSIGNED_INT, nullptr);
	}
	GLHandler::glf().glBindVertexArray(0);
}
