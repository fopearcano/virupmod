/*
    Copyright (C) 2019 Florian Cabot <florian.cabot@epfl.ch>

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

#include "Primitives.hpp"

void Primitives::setAsQuad(GLMesh& mesh, GLShaderProgram const& shader,
                           PrimitiveType primitiveType)
{
	const std::vector<float> vertices = {
	    -0.5f, -0.5f, // 0
	    -0.5f, 0.5f,  // 1
	    0.5f,  -0.5f, // 2
	    0.5f,  0.5f,  // 3
	};
	if(primitiveType == PrimitiveType::AUTO)
	{
		primitiveType = PrimitiveType::TRIANGLE_STRIP;
	}
	mesh.setPrimitiveType(primitiveType);

	if(primitiveType == PrimitiveType::POINTS)
	{
		mesh.setVertexShaderMapping(shader, {{"position", 2}});
		mesh.setVertices(vertices);
		return;
	}

	std::vector<unsigned int> elements;

	if(primitiveType == PrimitiveType::LINES)
	{
		elements = {0, 1, 1, 3, 3, 2, 2, 0};
	}
	else if(primitiveType == PrimitiveType::TRIANGLES)
	{
		elements = {0, 2, 1, 1, 2, 3};
	}
	else if(primitiveType == PrimitiveType::TRIANGLE_STRIP)
	{
		elements = {0, 2, 1, 3};
	}
	mesh.setVertexShaderMapping(shader, {{"position", 2}});
	mesh.setVertices(vertices, elements);
}

void Primitives::setAsGrid(GLMesh& mesh, GLShaderProgram const& shader,
                           unsigned int size, PrimitiveType primitiveType)
{
	std::vector<float> vertices;
	for(unsigned int i(0); i < size; ++i)
	{
		for(unsigned int j(0); j < size; ++j)
		{
			vertices.push_back(static_cast<float>(i) / (size - 1));
			vertices.push_back(static_cast<float>(j) / (size - 1));
		}
	}

	if(primitiveType == PrimitiveType::AUTO)
	{
		primitiveType = PrimitiveType::TRIANGLES;
	}
	mesh.setPrimitiveType(primitiveType);

	if(primitiveType == PrimitiveType::POINTS)
	{
		mesh.setVertexShaderMapping(shader, {{"position", 2}});
		mesh.setVertices(vertices);
		return;
	}

	// TODO(florian) finish this
	std::vector<unsigned int> elements;
	if(primitiveType == PrimitiveType::TRIANGLES)
	{
		for(unsigned int i(1); i < size; ++i)
		{
			for(unsigned int j(1); j < size; ++j)
			{
				const unsigned int id = j + size * i;

				elements.push_back(id - size - 1);
				elements.push_back(id - 1);
				elements.push_back(id - size);

				elements.push_back(id - 1);
				elements.push_back(id);
				elements.push_back(id - size);
			}
		}
	}
	else if(primitiveType == PrimitiveType::QUAD)
	{
		for(unsigned int i(1); i < size; ++i)
		{
			for(unsigned int j(1); j < size; ++j)
			{
				const unsigned int id = j + size * i;

				elements.push_back(id - size - 1);
				elements.push_back(id - size);
				elements.push_back(id);
				elements.push_back(id - 1);
			}
		}
	}
	mesh.setVertexShaderMapping(shader, {{"position", 2}});
	mesh.setVertices(vertices, elements);
}

void Primitives::setAsUnitCube(GLMesh& mesh, GLShaderProgram const& shader,
                               PrimitiveType primitiveType)
{
	const std::vector<float> vertices = {
	    -0.5f, -0.5f, -0.5f, // 0
	    -0.5f, -0.5f, 0.5f,  // 1
	    -0.5f, 0.5f,  -0.5f, // 2
	    -0.5f, 0.5f,  0.5f,  // 3
	    0.5f,  -0.5f, -0.5f, // 4
	    0.5f,  -0.5f, 0.5f,  // 5
	    0.5f,  0.5f,  -0.5f, // 6
	    0.5f,  0.5f,  0.5f,  // 7
	};

	if(primitiveType == PrimitiveType::AUTO)
	{
		primitiveType = PrimitiveType::TRIANGLE_STRIP;
	}
	mesh.setPrimitiveType(primitiveType);

	if(primitiveType == PrimitiveType::POINTS)
	{
		mesh.setVertexShaderMapping(shader, {{"position", 3}});
		mesh.setVertices(vertices);
		return;
	}

	std::vector<unsigned int> elements;

	if(primitiveType == PrimitiveType::LINES)
	{
		elements = {
		    0, 1, 0, 2, 0, 4,

		    7, 6, 7, 5, 7, 3,

		    1, 3, 1, 5,

		    2, 6, 2, 3,

		    4, 6, 4, 5,
		};
	}
	else if(primitiveType == PrimitiveType::TRIANGLES)
	{
		// as seen from (1, 0, 0) with up being (0, 0, 1)
		elements = {
		    0, 1, 2, // back0
		    3, 2, 1, // back1
		    6, 5, 4, // front0
		    7, 4, 5, // front1
		    0, 1, 4, // left0
		    5, 4, 1, // left1
		    2, 6, 3, // right0
		    7, 3, 6, // right1
		    0, 2, 4, // down0
		    6, 4, 2, // down1
		    1, 3, 5, // up0
		    7, 5, 3, // up1
		};
	}
	else if(primitiveType == PrimitiveType::TRIANGLE_STRIP)
	{
		// see : Optimizing Triangle Strips for Fast Rendering, Francine Evans,
		// Steven Skiena, Amitabh Varshney,
		// http://www.cs.umd.edu/gvil/papers/av_ts.pdf
		//
		// NOTATION :
		// their index : +/-(0.5), +/-(0.5), +/-(0.5) = our index
		//
		// if we define
		// 1 : -, -, - = 0
		// and their 1,3,8,5 to be the bottom face then :
		// 1 : -, -, - = 0
		// 2 : -, -, + = 1
		// 3 : +, -, - = 4
		// 4 : +, -, + = 5
		// 5 : -, +, - = 2
		// 6 : -, +, + = 3
		// 7 : +, +, + = 7
		// 8 : +, +, - = 6
		//
		// Their strip order is : 4 3 7 8 5 3 1 4 2 7 6 5 2 1,
		// thus :
		elements = {5, 4, 7, 6, 2, 4, 0, 5, 1, 7, 3, 2, 1, 0};
	}
	mesh.setVertexShaderMapping(shader, {{"position", 3}});
	mesh.setVertices(vertices, elements);
}

void Primitives::setAsUnitSphere(GLMesh& mesh, GLShaderProgram const& shader,
                                 unsigned int latDivisions,
                                 unsigned int lonDivisions,
                                 PrimitiveType primitiveType)
{
	std::vector<float> vertices;
	std::vector<unsigned int> elements;

	if(primitiveType == PrimitiveType::AUTO)
	{
		primitiveType = PrimitiveType::TRIANGLES;
	}
	mesh.setPrimitiveType(primitiveType);

	// "north pole"
	vertices.push_back(0.f);
	vertices.push_back(0.f);
	vertices.push_back(1.f);

	// link to first latitude
	if(primitiveType == PrimitiveType::TRIANGLES)
	{
		// all except last point
		for(unsigned int i(0); i < lonDivisions - 1; ++i)
		{
			elements.push_back(0);
			elements.push_back(i + 1);
			elements.push_back(i + 2);
		}
		elements.push_back(0);
		elements.push_back(lonDivisions);
		elements.push_back(1);
	}
	else if(primitiveType == PrimitiveType::LINES)
	{
		for(unsigned int i(0); i < lonDivisions; ++i)
		{
			elements.push_back(0);
			elements.push_back(i + 1);
		}
	}

	for(unsigned int i(0); i < latDivisions; ++i)
	{
		const float lat
		    = (static_cast<float>(i + 1) / (latDivisions + 1)) * M_PI;

		const float cosLat(std::cos(lat)), sinLat(std::sin(lat));

		for(unsigned int j(0); j < lonDivisions; ++j)
		{
			const float lon = 2 * M_PI * static_cast<float>(j) / lonDivisions;
			vertices.push_back(sinLat * std::cos(lon));
			vertices.push_back(sinLat * std::sin(lon));
			vertices.push_back(cosLat);

			// elements
			if(primitiveType == PrimitiveType::TRIANGLES)
			{
				// don't do anything on first latitude
				if(i == 0)
				{
					continue;
				}

				if(j != lonDivisions - 1) // if not last point
				{
					elements.push_back(latDivisions * i + j + 1);
					elements.push_back(latDivisions * i + j + 2);
					elements.push_back(latDivisions * (i - 1) + j + 1);

					elements.push_back(latDivisions * (i - 1) + j + 2);
					elements.push_back(latDivisions * (i - 1) + j + 1);
					elements.push_back(latDivisions * i + j + 2);
				}
				else
				{
					elements.push_back(latDivisions * i + j + 1);
					elements.push_back(latDivisions * i + 1);
					elements.push_back(latDivisions * (i - 1) + j + 1);

					elements.push_back(latDivisions * (i - 1) + 1);
					elements.push_back(latDivisions * (i - 1) + j + 1);
					elements.push_back(latDivisions * i + 1);
				}
			}
			else if(primitiveType == PrimitiveType::LINES)
			{
				// draw latitude line
				if(j != lonDivisions - 1) // if not last point
				{
					elements.push_back(latDivisions * i + j + 1);
					elements.push_back(latDivisions * i + j + 2);
				}
				else
				{
					elements.push_back(latDivisions * i + j + 1);
					elements.push_back(latDivisions * i + 1);
				}
				// draw longitude line
				if(i != 0) // if not first latitude
				{
					elements.push_back(latDivisions * i + j + 1);
					elements.push_back(latDivisions * (i - 1) + j + 1);
				}
			}
		}
	}

	// link last latitude to south pole
	const unsigned int southPole(latDivisions * lonDivisions + 1);
	if(primitiveType == PrimitiveType::TRIANGLES)
	{
		for(unsigned int i(0); i < lonDivisions - 1; ++i)
		{
			elements.push_back(southPole);
			elements.push_back(southPole - i - 1);
			elements.push_back(southPole - i - 2);
		}
		elements.push_back(southPole);
		elements.push_back(southPole - lonDivisions);
		elements.push_back(southPole - 1);
	}
	else if(primitiveType == PrimitiveType::LINES)
	{
		for(unsigned int i(0); i < lonDivisions; ++i)
		{
			elements.push_back(southPole);
			elements.push_back(southPole - i - 1);
		}
	}

	// "south pole"
	vertices.push_back(0.f);
	vertices.push_back(0.f);
	vertices.push_back(-1.f);

	mesh.setVertexShaderMapping(shader, {{"position", 3}});
	mesh.setVertices(vertices, elements);
}

void Primitives::setAsUnitCylinder(GLMesh& mesh, GLShaderProgram const& shader,
                                   unsigned int radialDivisions,
                                   PrimitiveType primitiveType)
{
	std::vector<float> vertices;
	std::vector<unsigned int> elements;

	if(primitiveType == PrimitiveType::AUTO)
	{
		primitiveType = PrimitiveType::TRIANGLES;
	}
	mesh.setPrimitiveType(primitiveType);

	// Generate cap center vertices
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);
	vertices.push_back(-1.0f);

	vertices.push_back(0.0f);
	vertices.push_back(0.0f);
	vertices.push_back(1.0f);

	// Generate side vertices
	for(int h = -1; h <= 1; h += 2)
	{
		for(unsigned int r = 0; r < radialDivisions; ++r)
		{
			const float angle
			    = 2.0f * M_PI * static_cast<float>(r) / radialDivisions;
			const float x = std::cos(angle);
			const float y = std::sin(angle);
			const auto z  = static_cast<float>(h);

			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);
		}
	}

	// Generate elements for side and caps
	for(unsigned int r = 0; r <= radialDivisions; ++r)
	{
		// NOLINTNEXTLINE(clang-analyzer-core.DivideZero)
		const unsigned int idx           = r % radialDivisions;
		const unsigned int sideIndex     = 2 + idx;
		const unsigned int next_r        = (r + 1) % radialDivisions;
		const unsigned int nextSideIndex = 2 + next_r;

		if(primitiveType == PrimitiveType::TRIANGLES)
		{
			// Side elements
			elements.push_back(sideIndex);
			elements.push_back(sideIndex + radialDivisions);
			elements.push_back(nextSideIndex + radialDivisions);

			elements.push_back(sideIndex);
			elements.push_back(nextSideIndex + radialDivisions);
			elements.push_back(nextSideIndex);

			// Cap elements
			if(r < radialDivisions)
			{
				elements.push_back(0);
				elements.push_back(sideIndex);
				elements.push_back(nextSideIndex);

				elements.push_back(1);
				elements.push_back(nextSideIndex + radialDivisions);
				elements.push_back(sideIndex + radialDivisions);
			}
		}
		else if(primitiveType == PrimitiveType::LINES)
		{
			elements.push_back(sideIndex);
			elements.push_back(sideIndex + radialDivisions);

			elements.push_back(sideIndex);
			elements.push_back(nextSideIndex);

			// Cap elements
			if(r < radialDivisions)
			{
				elements.push_back(0);
				elements.push_back(sideIndex);

				elements.push_back(1);
				elements.push_back(sideIndex + radialDivisions);
			}
		}
	}

	/*if(primitiveType == PrimitiveType::TRIANGLE_STRIP)
	{
	    // Bottom cap
	    for(unsigned int r = 0; r <= radialDivisions; ++r)
	    {
	        unsigned int idx       = r % radialDivisions;
	        unsigned int sideIndex = 2 + idx;

	        if(radialDivisions % 2 == 0)
	        {
	            if(r % 2 == 0)
	            {
	                elements.push_back(0);
	            }
	            else
	            {
	                elements.push_back(sideIndex);
	            }
	        }
	        else
	        {
	            if(r % 2 == 0)
	            {
	                elements.push_back(sideIndex);
	            }
	            else
	            {
	                elements.push_back(0);
	            }
	        }
	    }

	    // Separator degenerate triangles
	    elements.push_back(2);
	    elements.push_back(2 + radialDivisions);

	    // Sides
	    for(unsigned int r = 0; r <= radialDivisions; ++r)
	    {
	        unsigned int idx       = r % radialDivisions;
	        unsigned int sideIndex = 2 + idx;

	        elements.push_back(sideIndex);
	        elements.push_back(sideIndex + radialDivisions);
	    }

	    // Separator degenerate triangles
	    elements.push_back(1 + radialDivisions);
	    elements.push_back(1);

	    // Top cap
	    for(unsigned int r = 0; r <= radialDivisions; ++r)
	    {
	        unsigned int idx       = r % radialDivisions;
	        unsigned int sideIndex = 2 + idx + radialDivisions;

	        if(radialDivisions % 2 == 0)
	        {
	            if(r % 2 == 0)
	            {
	                elements.push_back(1);
	            }
	            else
	            {
	                elements.push_back(sideIndex);
	            }
	        }
	        else
	        {
	            if(r % 2 == 0)
	            {
	                elements.push_back(sideIndex);
	            }
	            else
	            {
	                elements.push_back(1);
	            }
	        }
	    }
	}*/

	mesh.setVertexShaderMapping(shader, {{"position", 3}});
	mesh.setVertices(vertices, elements);
}
