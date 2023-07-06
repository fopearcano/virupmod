/*
    Copyright (C) 2019 Florian Cabot <florian.cabot@hotmail.fr>

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

#include "AsyncMesh.hpp"

bool& AsyncMesh::forceSync()
{
	static bool forceSync(false);
	return forceSync;
}

AsyncMesh::AsyncMesh(QString const& path, GLMesh&& defaultMesh)
    : defaultMesh(std::move(defaultMesh))
{
	if(path.isEmpty())
	{
		emptyPath = true;
		return;
	}

	future
	    = QtConcurrent::run([path]() { return AssetLoader::loadFile(path); });
}

void AsyncMesh::updateMesh(GLShaderProgram const& shader)
{
	if(emptyPath || loaded)
	{
		return;
	}

	if(!future.isFinished())
	{
		if(forceSync())
		{
			future.waitForFinished();
		}
		else
		{
			return;
		}
	}

	loaded               = true;
	auto pair            = future.result();
	boundingSphereRadius = pair.first;

	std::vector<AssetLoader::TexturedMesh> meshes
	    = AssetLoader::loadModel(pair.second, shader);

	if(meshes.size() == 1)
	{
		mesh = std::move(meshes[0].mesh);
	}

	future = {};
}

GLMesh const& AsyncMesh::getMesh()
{
	if(emptyPath || !loaded)
	{
		return defaultMesh;
	}
	return mesh;
}
