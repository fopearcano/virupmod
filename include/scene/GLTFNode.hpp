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

#ifndef GLTFNODE_HPP
#define GLTFNODE_HPP

#include "gltf/GPUData.hpp"
#include "scene/Node.hpp"

class GLTFNode : public Node
{
  public:
	GLTFNode(QString src, std::map<QString, Node*>& nodesDict);
	virtual std::vector<std::pair<GLMesh const&, QMatrix4x4>>
	    getShadowCastingMeshes() const override;
	virtual bool setTransform(QString const& subNodeName,
	                          QMatrix4x4 const& transform) override;

  protected:
	QMatrix4x4 preMultiplyTransform() const override;
	void doRender(BasicCamera const& cam,
	              std::vector<Light const*> const& lights,
	              GLTexture const& brdfLUT) override;

  private:
	void loadCPU();
	void loadGPU();

	QString src;
	QMatrix4x4 gltfModel;

	gltf::CPUData cpuData;
	std::unique_ptr<gltf::GPUData> gpuData;
};

#endif // GLTFNODE_HPP
