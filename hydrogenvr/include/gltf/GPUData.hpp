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

#ifndef GLTFGPUDATA_HPP
#define GLTFGPUDATA_HPP

#include "PBRMaterial.hpp"
#include "camera/BoundingVolumes.hpp"
#include "gltf/CPUData.hpp"

class BasicCamera;
class Light;

namespace gltf
{
struct GPUMesh
{
	struct Primitive
	{
		Primitive(gltf::Mesh::Primitive const& prim, QString const& meshName);
		void render(BasicCamera const& cam,
		            std::vector<Light const*> const& lights,
		            QMatrix4x4 const& nodeModel, GLTexture const& irradiance,
		            GLTexture const& prefiltered,
		            GLTexture const& brdfLUT) const;
		GLMesh mesh;
		std::vector<QPair<QString, std::vector<float>>> unusedAttrVec;
		std::unique_ptr<PBRMaterial> material;
		BoundingSphere boundingSphere;

	  private:
		// returns vertex attrib for appended normals
		GLMesh::VertexAttrib
		    computeNormals(gltf::Accessor const& positionAccessor,
		                   gltf::Accessor const* indicesAccessor,
		                   std::vector<float>& newBuffer);
	};

	GPUMesh(gltf::Mesh const& gltfmesh);
	void render(BasicCamera const& cam, std::vector<Light const*> const& lights,
	            QMatrix4x4 const& nodeModel, GLTexture const& irradiance,
	            GLTexture const& prefiltered, GLTexture const& brdfLUT) const;
	std::vector<Primitive> primitives;
	std::vector<GLMesh const*> glMeshes;
	BoundingSphere boundingSphere;
};
struct GPUNode
{
	GPUNode(gltf::Node const& node, std::map<QString, GPUNode*>& nodesDict);
	GPUNode(GPUNode const& other)            = delete;
	GPUNode& operator=(GPUNode const& other) = delete;
	GPUNode(GPUNode&& moved) noexcept;
	GPUNode& operator=(GPUNode&& moved) = delete;

	std::vector<std::pair<GLMesh const&, QMatrix4x4>>
	    getMeshes(QMatrix4x4 const& parentNodeModel) const;
	BoundingSphere getBoundingSphere() const;
	void render(BasicCamera const& cam, std::vector<Light const*> const& lights,
	            QMatrix4x4 const& parentNodeModel, GLTexture const& irradiance,
	            GLTexture const& prefiltered, GLTexture const& brdfLUT) const;
	std::unique_ptr<GPUMesh> gpuMesh;
	QString name;
	const QMatrix4x4 baseModel;
	QMatrix4x4 transform;
	std::vector<GPUNode> children;

	// keep a ref to update on copy
	std::map<QString, GPUNode*>& nodesDict;
};
struct GPUData
{
	GPUData(gltf::CPUData const& cpuData);
	std::vector<GPUNode> nodes;
	std::map<QString, GPUNode*> nodesDict;
};
} // namespace gltf

#endif // GLTFGPUDATA_HPP
