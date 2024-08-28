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

#include "gltf/GPUData.hpp"

#include "Light.hpp"
#include "camera/BasicCamera.hpp"

namespace gltf
{
GPUMesh::Primitive::Primitive(gltf::Mesh::Primitive const& prim,
                              QString const& meshName)
{
	auto albedoSpec(std::make_unique<PBRMaterial::AlbedoSpec>(
	    prim.material->pbrMetallicRoughness.baseColorFactor));
	auto tex = prim.material->pbrMetallicRoughness.baseColorTexture;
	if(tex != nullptr)
	{
		albedoSpec = std::make_unique<PBRMaterial::AlbedoSpec>(
		    tex->source->data, tex->sampler->glSampler);
	}
	auto occlusionSpec = std::make_unique<PBRMaterial::OcclusionSpec>(
	    1.f, prim.material->occlusionStrength);
	tex = prim.material->occlusionTexture;
	if(tex != nullptr)
	{
		occlusionSpec = std::make_unique<PBRMaterial::OcclusionSpec>(
		    tex->source->data, tex->sampler->glSampler,
		    prim.material->occlusionStrength);
	}
	auto emissiveSpec = std::make_unique<PBRMaterial::EmissiveSpec>(
	    QColor(0, 0, 0), prim.material->emissiveFactor);
	tex = prim.material->emissiveTexture;
	if(tex != nullptr)
	{
		emissiveSpec = std::make_unique<PBRMaterial::EmissiveSpec>(
		    tex->source->data, tex->sampler->glSampler,
		    prim.material->emissiveFactor);
	}
	auto mRSpec(std::make_unique<PBRMaterial::MetallicRoughnessSpec>(
	    prim.material->pbrMetallicRoughness.metallicFactor,
	    prim.material->pbrMetallicRoughness.roughnessFactor));
	tex = prim.material->pbrMetallicRoughness.metallicRoughnessTexture;
	if(tex != nullptr)
	{
		mRSpec = std::make_unique<PBRMaterial::MetallicRoughnessSpec>(
		    tex->source->data, tex->sampler->glSampler);
	}
	auto normalSpec(std::make_unique<PBRMaterial::NormalSpec>());
	tex = prim.material->normalTexture;
	if(tex != nullptr)
	{
		normalSpec = std::make_unique<PBRMaterial::NormalSpec>(
		    tex->source->data, tex->sampler->glSampler);
	}
	material = std::make_unique<PBRMaterial>(
	    *albedoSpec, *occlusionSpec, *emissiveSpec, *mRSpec, *normalSpec);
	if(prim.material->alphaMode == Material::AlphaMode::MASK)
	{
		material->setAlphaMode(PBRMaterial::AlphaMode::MASK);
		material->setAlphaCutoff(prim.material->alphaCutoff);
	}
	if(prim.material->alphaMode == Material::AlphaMode::BLEND)
	{
		material->setAlphaMode(PBRMaterial::AlphaMode::BLEND);
	}
	material->setDoubleSided(prim.material->doubleSided);

	// set bounding sphere
	auto const &minVec(prim.attributes.at("POSITION")->min),
	    &maxVec(prim.attributes.at("POSITION")->max);
	QVector3D min(minVec[0], minVec[1], minVec[2]),
	    max(maxVec[0], maxVec[1], maxVec[2]);
	boundingSphere = {0.5f * (min + max), 0.5f * (max - min).length()};

	mesh.setPrimitiveType(prim.mode);

	// TODO(florian) : make sure we can assume all attribute accessors share the
	// same buffer and bufferViews are contiguous
	QMap<QString, QVector3D> unusedAttributes;
	unusedAttributes["position"] = QVector3D();
	unusedAttributes["tangent"]  = QVector3D(100.f, 0.f, 0.f);
	unusedAttributes["normal"]   = QVector3D();
	QColor c                     = GLHandler::sRGBToLinear(
        prim.material->pbrMetallicRoughness.baseColorFactor);
	unusedAttributes["color_0"]    = QVector3D(c.redF(), c.greenF(), c.blueF());
	unusedAttributes["texcoord_0"] = QVector3D();

	int globalOffset  = INT_MAX;
	size_t globalSize = 0;
	std::vector<gltf::BufferView const*> alreadySeen;
	for(auto const& pair : prim.attributes)
	{
		if(globalOffset > pair.second->bufferView->byteOffset)
		{
			globalOffset = pair.second->bufferView->byteOffset;
		}
		bool seen = false;
		for(auto bv : alreadySeen)
		{
			if(bv == pair.second->bufferView)
			{
				seen = true;
				break;
			}
		}
		if(!seen)
		{
			alreadySeen.push_back(pair.second->bufferView);
			globalSize += pair.second->bufferView->byteLength;
		}
	}

	std::vector<GLMesh::VertexAttrib> mapping;
	for(auto const& pair : prim.attributes)
	{
		QString name(pair.first.toLower());
		unusedAttributes.remove(name);
		int size(pair.second->typeDimensions());
		if(name == "color_0" && size > 3)
		{
			qWarning() << "Attribute color_0 is of dimension" << size
			           << "but only size 3 is supported for now.";
		}
		size_t stride(pair.second->bufferView->byteStride / sizeof(float));
		size_t offset((pair.second->bufferView->byteOffset - globalOffset)
		              + pair.second->byteOffset);
		offset /= sizeof(float);
		mapping.emplace_back(name, size, stride, offset);
	}

	auto const& view = *prim.attributes.at("POSITION")->bufferView;
	// hardcoded float; GLushort and * 1
	auto vertexDataBuff
	    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	    = reinterpret_cast<float const*>(&view.buffer.data[globalOffset]);
	auto vertexDataSize = globalSize / sizeof(float);

	// compute face normals if not present
	std::vector<float> res;
	if(unusedAttributes.contains("normal"))
	{
		auto va = computeNormals(*prim.attributes.at("POSITION"), prim.indices,
		                         res);
		vertexDataBuff = &res[0];
		vertexDataSize = res.size();
		mapping.push_back(va);
		unusedAttributes.remove("normal");
	}
	if(unusedAttributes.contains("tangent")
	   && prim.material->normalTexture != nullptr)
	{
		qWarning()
		    << "Mesh" << meshName
		    << "glTF model doesn't contain tangents. The way they will "
		       "be computed might be wrong in the current state of the "
		       "implementation. A quick fix for this would be to open the "
		       "model in Blender and export it again including tangents.";
	}

	mesh.setVertexShaderMapping(material->getShader(), mapping);
	for(auto const& key : unusedAttributes.keys())
	{
		unusedAttrVec.push_back(
		    {key,
		     {unusedAttributes[key].x(), unusedAttributes[key].y(),
		      unusedAttributes[key].z()}});
	}

	if(prim.indices == nullptr)
	{
		mesh.setVertices(vertexDataBuff, vertexDataSize);
	}
	else
	{
		auto const& accessInd = *prim.indices;
		auto const& viewInd   = *accessInd.bufferView;
		auto const& buffInd   = viewInd.buffer;

		// TODO(florian) not convert to std::vector<unsigned int>
		std::vector<unsigned int> elements;
		elements.reserve(accessInd.count * 1);
		switch(accessInd.componentType)
		{
			case gltf::Accessor::ComponentType::UNSIGNED_SHORT:
			{
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
				auto indBuff = reinterpret_cast<GLushort const*>(
				    &buffInd.data[viewInd.byteOffset + accessInd.byteOffset]);
				for(int i(0); i < accessInd.count * 1; ++i)
				{
					elements.push_back(indBuff[i]);
				}
			}
			break;
			case gltf::Accessor::ComponentType::UNSIGNED_INT:
			{
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
				auto indBuff = reinterpret_cast<GLuint const*>(
				    &buffInd.data[viewInd.byteOffset + accessInd.byteOffset]);
				for(int i(0); i < accessInd.count * 1; ++i)
				{
					elements.push_back(indBuff[i]);
				}
			}
			break;
			default:
				qWarning() << "Unhandled indices type :"
				           << accessInd.componentType;
		}

		mesh.setVertices(vertexDataBuff, vertexDataSize, &elements[0],
		                 accessInd.count * 1);
	}
}

void GPUMesh::Primitive::render(BasicCamera const& cam,
                                std::vector<Light const*> const& lights,
                                QMatrix4x4 const& nodeModel,
                                GLTexture const& irradiance,
                                GLTexture const& prefiltered,
                                GLTexture const& brdfLUT) const
{
	std::unordered_map<int, bool> stateSet;
	stateSet[GL_TEXTURE_CUBE_MAP_SEAMLESS] = true;
	if(material->isDoubleSided())
	{
		stateSet[GL_CULL_FACE] = false;
	}
	GLStateSet glState(stateSet);
	// could be overwritten between different materials
	material->getShader().setUnusedAttributesValues(unusedAttrVec);
	material->getShader().setUniform("debug", 1);
	material->update(nodeModel, cam.getWorldSpacePosition(), lights);
	std::vector<GLTexture const*> shadowmaps;
	shadowmaps.reserve(lights.size());
	for(auto light : lights)
	{
		shadowmaps.push_back(&light->getShadowMap());
	}
	material->setUpTextures(irradiance, prefiltered, brdfLUT, shadowmaps);
	GLHandler::setUpRender(material->getShader(), nodeModel);
	if(material->getAlphaMode() != PBRMaterial::AlphaMode::OPAQUE)
	{
		GLBlendSet glBlend(GLBlendSet::BlendState{});
		mesh.render();
	}
	else
	{
		mesh.render();
	}
}

GLMesh::VertexAttrib
    GPUMesh::Primitive::computeNormals(gltf::Accessor const& positionAccessor,
                                       gltf::Accessor const* indicesAccessor,
                                       std::vector<float>& newBuffer)
{
	std::vector<unsigned int> indices;
	if(indicesAccessor != nullptr)
	{
		auto const& viewInd = *indicesAccessor->bufferView;
		auto const& buffInd = viewInd.buffer;

		// TODO(florian) not convert to std::vector<unsigned int>
		std::vector<unsigned int> elements;
		elements.reserve(indicesAccessor->count * 1);
		switch(indicesAccessor->componentType)
		{
			case gltf::Accessor::ComponentType::UNSIGNED_SHORT:
			{
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
				auto indBuff = reinterpret_cast<GLushort const*>(
				    &buffInd.data[viewInd.byteOffset
				                  + indicesAccessor->byteOffset]);
				for(int i(0); i < indicesAccessor->count * 1; ++i)
				{
					elements.push_back(indBuff[i]);
				}
			}
			break;
			case gltf::Accessor::ComponentType::UNSIGNED_INT:
			{
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
				auto indBuff = reinterpret_cast<GLuint const*>(
				    &buffInd.data[viewInd.byteOffset
				                  + indicesAccessor->byteOffset]);
				for(int i(0); i < indicesAccessor->count * 1; ++i)
				{
					elements.push_back(indBuff[i]);
				}
			}
			break;
			default:
				qWarning() << "Unhandled indices type :"
				           << indicesAccessor->componentType;
		}
	}
	else
	{
		indices.reserve(positionAccessor.count);
		for(int i(0); i < positionAccessor.count; ++i)
		{
			indices.push_back(i);
		}
	}

	// copy buffer view first
	newBuffer.reserve(positionAccessor.bufferView->byteLength / sizeof(float));
	auto const& bufferView = *positionAccessor.bufferView;
	auto const& buffer     = bufferView.buffer;
	for(unsigned int i(0);
	    i < positionAccessor.bufferView->byteLength / sizeof(float); ++i)
	{
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		newBuffer.push_back(*reinterpret_cast<float const*>(
		    &buffer.data[bufferView.byteOffset + i * sizeof(float)]));
	}

	// works only for TRIANGLES
	// iterate through triangles
	unsigned int oldSize(newBuffer.size());
	newBuffer.resize(oldSize + positionAccessor.count * 3);

	int offset = positionAccessor.byteOffset / sizeof(float);
	int stride = bufferView.byteStride / sizeof(float);
	if(stride == 0)
	{
		stride = 3;
	}
	for(unsigned int i(0); i < indices.size(); i += 3)
	{
		int idx0 = offset + indices[i] * stride;
		int idx1 = offset + indices[i + 1] * stride;
		int idx2 = offset + indices[i + 2] * stride;
		QVector3D pos0(newBuffer[idx0], newBuffer[idx0 + 1],
		               newBuffer[idx0 + 2]);
		QVector3D pos1(newBuffer[idx1], newBuffer[idx1 + 1],
		               newBuffer[idx1 + 2]);
		QVector3D pos2(newBuffer[idx2], newBuffer[idx2 + 1],
		               newBuffer[idx2 + 2]);

		QVector3D normal = QVector3D::crossProduct(pos1 - pos0, pos2 - pos0);
		// assign normal three times ; face normal
		int idxn0            = oldSize + indices[i] * stride;
		int idxn1            = oldSize + indices[i + 1] * stride;
		int idxn2            = oldSize + indices[i + 2] * stride;
		newBuffer[idxn0]     = normal[0];
		newBuffer[idxn1]     = normal[0];
		newBuffer[idxn2]     = normal[0];
		newBuffer[idxn0 + 1] = normal[1];
		newBuffer[idxn1 + 1] = normal[1];
		newBuffer[idxn2 + 1] = normal[1];
		newBuffer[idxn0 + 2] = normal[2];
		newBuffer[idxn1 + 2] = normal[2];
		newBuffer[idxn2 + 2] = normal[2];
	}
	return GLMesh::VertexAttrib{"normal", 3, 3,
	                            positionAccessor.bufferView->byteLength
	                                / sizeof(float)};
}

GPUMesh::GPUMesh(gltf::Mesh const& gltfmesh)
{
	boundingSphere = {{}, 0.f};
	for(auto const& prim : gltfmesh.primitives)
	{
		primitives.emplace_back(prim, gltfmesh.name);
		boundingSphere
		    = boundingSphere.merged(primitives.back().boundingSphere);
		glMeshes.emplace_back(&primitives.back().mesh);
	}
}

void GPUMesh::render(BasicCamera const& cam,
                     std::vector<Light const*> const& lights,
                     QMatrix4x4 const& nodeModel, GLTexture const& irradiance,
                     GLTexture const& prefiltered,
                     GLTexture const& brdfLUT) const
{
	for(auto const& prim : primitives)
	{
		prim.render(cam, lights, nodeModel, irradiance, prefiltered, brdfLUT);
	}
}

GPUNode::GPUNode(gltf::Node const& node, std::map<QString, GPUNode*>& nodesDict)
    : baseModel(node.matrix)
    , nodesDict(nodesDict)
{
	if(node.mesh != nullptr)
	{
		gpuMesh = std::make_unique<GPUMesh>(*node.mesh);
	}
	name = node.name;
	if(!name.isEmpty())
	{
		nodesDict[name] = this;
	}
	for(auto child : node.children)
	{
		children.emplace_back(*child, nodesDict);
	}
}

GPUNode::GPUNode(GPUNode&& moved) noexcept
    : gpuMesh(std::move(moved.gpuMesh))
    , name(std::move(moved.name))
    , baseModel(moved.baseModel)
    , transform(moved.transform)
    , children(std::move(moved.children))
    , nodesDict(moved.nodesDict)
{
	if(!name.isEmpty())
	{
		nodesDict[name] = this;
	}
}

std::vector<std::pair<GLMesh const&, QMatrix4x4>>
    GPUNode::getMeshes(QMatrix4x4 const& parentNodeModel) const
{
	std::vector<std::pair<GLMesh const&, QMatrix4x4>> result;
	if(gpuMesh != nullptr)
	{
		for(auto const& glMesh : gpuMesh->glMeshes)
		{
			result.emplace_back(*glMesh,
			                    parentNodeModel * baseModel * transform);
		}
	}
	for(auto const& child : children)
	{
		for(auto const& pair :
		    child.getMeshes(parentNodeModel * baseModel * transform))
		{
			result.push_back(pair);
		}
	}
	return result;
}

BoundingSphere GPUNode::getBoundingSphere() const
{
	BoundingSphere result{{}, 0.f};
	if(gpuMesh != nullptr)
	{
		result = gpuMesh->boundingSphere.transformed(baseModel * transform);
	}
	for(auto const& child : children)
	{
		auto childBS(
		    child.getBoundingSphere().transformed(baseModel * transform));
		if(result.radius == 0.f)
		{
			result = childBS;
		}
		else
		{
			result = result.merged(childBS);
		}
	}
	return result;
}

void GPUNode::render(BasicCamera const& cam,
                     std::vector<Light const*> const& lights,
                     QMatrix4x4 const& parentNodeModel,
                     GLTexture const& irradiance, GLTexture const& prefiltered,
                     GLTexture const& brdfLUT) const
{
	if(gpuMesh != nullptr)
	{
		gpuMesh->render(cam, lights, parentNodeModel * baseModel * transform,
		                irradiance, prefiltered, brdfLUT);
	}
	for(auto const& child : children)
	{
		child.render(cam, lights, parentNodeModel * baseModel * transform,
		             irradiance, prefiltered, brdfLUT);
	}
}

GPUData::GPUData(gltf::CPUData const& cpuData)
{
	if(cpuData.scene != nullptr)
	{
		for(auto const& node : cpuData.scene->nodes)
		{
			nodes.emplace_back(*node, nodesDict);
		}
		return;
	}
	if(!cpuData.scenes.empty())
	{
		for(auto const& node : cpuData.scenes.front().nodes)
		{
			nodes.emplace_back(*node, nodesDict);
		}
	}
	for(auto const& node : cpuData.nodes)
	{
		nodes.emplace_back(node, nodesDict);
	}
}

} // namespace gltf
