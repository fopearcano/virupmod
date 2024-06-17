/*
    Copyright (C) 2024 Florian Cabot <florian.cabot@hotmail.fr>

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

#include "scene/AssimpNode.hpp"

AssimpNode::AssimpNode(QString const& src, std::map<QString, Node*>& nodesDict)
    : Node(nodesDict, src.split('/').last())
    , model(src)
{
	boundingSphere = {{}, model.getBoundingSphereRadius()};
}

std::vector<std::pair<GLMesh const&, QMatrix4x4>>
    AssimpNode::getShadowCastingMeshes() const
{
	std::vector<std::pair<GLMesh const&, QMatrix4x4>> result;
	for(auto const& pair : model.getMeshes())
	{
		result.emplace_back(pair.first, getModel() * pair.second);
	}
	return result;
}

QMatrix4x4 AssimpNode::preMultiplyTransform() const
{
	QMatrix4x4 result;
	result.rotate(90.f, QVector3D(0.f, 0.f, 1.f));
	result.rotate(90.f, QVector3D(1.f, 0.f, 0.f));
	return result;
}

void AssimpNode::doRender(BasicCamera const& cam,
                          std::vector<Light const*> const& lights,
                          GLTexture const& /*brdfLUT*/)
{
	model.render(cam.getWorldSpacePosition(), getModel(), lights);
}
