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

#include "gltf/CPUData.hpp"

#include <QJsonArray>
#include <QJsonObject>

namespace gltf
{

Buffer::Buffer(QJsonObject const& json, std::vector<char>& glbBinBufferChunk)
{
	for(auto const& key : json.keys())
	{
		if(key != "uri" && key != "byteLength" && key != "name")
		{
			qWarning() << "gltf::Buffer key" << key
			           << "parsing not implemented. In this case contains:"
			           << json[key];
		}
	}
	const QString uri    = json["uri"].toString();
	const int byteLength = json["byteLength"].toInt();
	data.resize(byteLength);
	name = json["name"].toString();

	if(uri == "")
	{
		data = std::move(glbBinBufferChunk);
		return;
	}

	QFile file(uri); // TODO(florian): solve when this is over network
	file.open(QIODevice::ReadOnly);
	file.read(data.data(), byteLength);
}

BufferView::BufferView(QJsonObject const& json,
                       std::vector<Buffer> const& globalBuffers)
    : buffer(loadBuffer(json, globalBuffers))
{
	for(auto const& key : json.keys())
	{
		if(key != "buffer" && key != "byteOffset" && key != "byteLength"
		   && key != "byteStride" && key != "target" && key != "name")
		{
			qWarning() << "gltf::BufferView key" << key
			           << "parsing not implemented. In this case contains:"
			           << json[key];
		}
	}
	byteOffset = json["byteOffset"].toInt();
	byteLength = json["byteLength"].toInt();
	byteStride = json["byteStride"].toInt();
	target     = static_cast<Target>(json["target"].toInt());
	name       = json["name"].toString();
}

Buffer const& BufferView::loadBuffer(QJsonObject const& json,
                                     std::vector<Buffer> const& globalBuffers)
{
	const int idx = json["buffer"].toInt(-1);
	if(idx >= 0)
	{
		return globalBuffers[idx];
	}
	return globalBuffers.back();
}

void Texture::load(QJsonObject const& json,
                   std::vector<Sampler> const& globalSamplers,
                   std::vector<Image> const& globalImages)
{
	for(auto const& key : json.keys())
	{
		if(key != "sampler" && key != "source")
		{
			qWarning() << "gltf::Texture key" << key
			           << "parsing not implemented. In this case contains:"
			           << json[key];
		}
	}
	int idx = json["sampler"].toInt(-1);
	if(idx >= 0)
	{
		sampler = &globalSamplers[idx];
	}
	else
	{
		sampler = &globalSamplers.back();
	}
	idx = json["source"].toInt(-1);
	if(idx >= 0)
	{
		source = &globalImages[idx];
	}
}

void Accessor::load(QJsonObject const& json,
                    std::vector<BufferView> const& globalBufferViews)
{
	for(auto const& key : json.keys())
	{
		if(key != "byteOffset" && key != "componentType" && key != "count"
		   && key != "type" && key != "max" && key != "min"
		   && key != "bufferView" && key != "name")
		{
			qWarning() << "gltf::Accessor key" << key
			           << "parsing not implemented. In this case contains:"
			           << json[key];
		}
	}
	const int idx = json["bufferView"].toInt(-1);
	if(idx >= 0)
	{
		bufferView = &globalBufferViews[idx];
	}
	byteOffset    = json["byteOffset"].toInt();
	componentType = static_cast<ComponentType>(json["componentType"].toInt());
	count         = json["count"].toInt();
	type          = json["type"].toString();
	max.reserve(typeDimensions());
	min.reserve(typeDimensions());
	name = json["name"].toString();
	for(auto const& valJSON : json["max"].toArray())
	{
		max.push_back(valJSON.toDouble());
	}
	for(auto const& valJSON : json["min"].toArray())
	{
		min.push_back(valJSON.toDouble());
	}
}

int Accessor::sizeInBytes(ComponentType type)
{
	switch(type)
	{
		case ComponentType::BYTE:
		case ComponentType::UNSIGNED_BYTE:
			return sizeof(char);
		case ComponentType::SHORT:
		case ComponentType::UNSIGNED_SHORT:
			return sizeof(GLshort);
		case ComponentType::UNSIGNED_INT:
		case ComponentType::FLOAT:
			return sizeof(float);
	}
	return -1;
}

int Accessor::typeDimensions() const
{
	if(type == "SCALAR")
	{
		return 1;
	}
	if(type == "VEC2")
	{
		return 2;
	}
	if(type == "VEC3")
	{
		return 3;
	}
	if(type == "VEC4" || type == "MAT2")
	{
		return 4;
	}
	if(type == "MAT3")
	{
		return 9;
	}
	if(type == "MAT4")
	{
		return 16;
	}
	qWarning() << "Unknown type" << type;
	return -1;
}

Sampler::Sampler(QJsonObject const& json)
{
	for(auto const& key : json.keys())
	{
		if(key != "magFilter" && key != "minFilter" && key != "wrapS"
		   && key != "wrapT" && key != "name")
		{
			qWarning() << "gltf::Sampler key" << key
			           << "parsing not implemented. In this case contains:"
			           << json[key];
		}
	}
	const GLint magFilter = convertFilter(json["magFilter"].toInt(9728));
	const GLint minFilter = convertFilter(json["minFilter"].toInt(9728));
	const GLint wrapS     = convertWrap(json["wrapS"].toInt(10497));
	const GLint wrapT     = convertWrap(json["wrapT"].toInt(10497));
	glSampler             = GLTexture::Sampler(magFilter, wrapS, wrapT);
	glSampler.minfilter   = minFilter;
	name                  = json["name"].toString();
}

GLint Sampler::convertFilter(int filter)
{
	switch(filter)
	{
		case 9728:
			return GL_NEAREST;
		case 9729:
			return GL_LINEAR;
		case 9984:
			return GL_NEAREST_MIPMAP_NEAREST;
		case 9985:
			return GL_LINEAR_MIPMAP_NEAREST;
		case 9986:
			return GL_NEAREST_MIPMAP_LINEAR;
		case 9987:
			return GL_LINEAR_MIPMAP_LINEAR;
		default:
			break;
	}
	qWarning() << "Unknown filter" << filter;
	return -1;
}

GLint Sampler::convertWrap(int wrap)
{
	switch(wrap)
	{
		case 33071:
			return GL_CLAMP_TO_EDGE;
		case 33648:
			return GL_MIRRORED_REPEAT;
		case 10497:
			return GL_REPEAT;
		default:
			break;
	}
	qWarning() << "Unknown wrap mode" << wrap;
	return -1;
}

Image::Image(QJsonObject const& json,
             std::vector<BufferView> const& globalBufferViews)
{
	for(auto const& key : json.keys())
	{
		if(key != "uri" && key != "name" && key != "mimeType"
		   && key != "bufferView")
		{
			qWarning() << "gltf::Image key" << key
			           << "parsing not implemented. In this case contains:"
			           << json[key];
		}
	}
	if(json.contains("uri"))
	{
		const QString uri      = json["uri"].toString();
		const QString mimeType = json["mimeType"].toString();
		data.load(
		    uri,
		    mimeType
		        .toLatin1()); // TODO(florian): solve when this is over network
	}
	else
	{
		const int idx = json["bufferView"].toInt(-1);
		if(idx >= 0)
		{
			auto const& bufferView = globalBufferViews[idx];
			const QString mimeType = json["mimeType"].toString();
			auto const* dataPtr
			    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			    = reinterpret_cast<uchar const*>(bufferView.buffer.data.data());
			dataPtr += bufferView.byteOffset;
			data.loadFromData(dataPtr, bufferView.byteLength,
			                  mimeType.toLatin1());
		}
	}

	name = json["name"].toString();
}

void Material::PBRMetallicRoughness::load(
    QJsonObject const& json, std::vector<Texture> const& globalTextures)
{
	for(auto const& key : json.keys())
	{
		if(key != "baseColorFactor" && key != "baseColorTexture"
		   && key != "metallicFactor" && key != "roughnessFactor"
		   && key != "metallicRoughnessTexture")
		{
			qWarning() << "gltf::GLTFMesh::Primitive key" << key
			           << "parsing not implemented. In this case contains:"
			           << json[key];
		}
	}
	if(json.contains("baseColorFactor"))
	{
		QColor c;
		c.setRedF(json["baseColorFactor"].toArray()[0].toDouble(1.0));
		c.setGreenF(json["baseColorFactor"].toArray()[1].toDouble(1.0));
		c.setBlueF(json["baseColorFactor"].toArray()[2].toDouble(1.0));
		c.setAlphaF(json["baseColorFactor"].toArray()[3].toDouble(1.0));
		if(c.alphaF() != 1.f)
		{
			qWarning() << "Non 1.0 alpha specified for baseColorFactor. This "
			              "is unsupported as of now.";
		}
		baseColorFactor = c;
	}
	for(auto const& key : json["baseColorTexture"].toObject().keys())
	{
		if(key != "index")
		{
			qWarning()
			    << "gltf::Material::PBRMetallicRoughness::baseColorTexture key"
			    << key << "parsing not implemented. In this case contains:"
			    << json[key];
		}
	}
	int idx = json["baseColorTexture"].toObject()["index"].toInt(-1);
	if(idx >= 0)
	{
		baseColorTexture = &globalTextures[idx];
	}
	metallicFactor  = json["metallicFactor"].toDouble(1.0);
	roughnessFactor = json["roughnessFactor"].toDouble(1.0);
	for(auto const& key : json["metallicRoughnessTexture"].toObject().keys())
	{
		if(key != "index")
		{
			qWarning() << "gltf::Material::PBRMetallicRoughness::"
			              "metallicRoughnessTexture key"
			           << key
			           << "parsing not implemented. In this case contains:"
			           << json[key];
		}
	}
	idx = json["metallicRoughnessTexture"].toObject()["index"].toInt(-1);
	if(idx >= 0)
	{
		metallicRoughnessTexture = &globalTextures[idx];
	}
}

void Material::load(QJsonObject const& json,
                    std::vector<Texture> const& globalTextures)
{
	for(auto const& key : json.keys())
	{
		if(key != "name" && key != "pbrMetallicRoughness"
		   && key != "normalTexture" && key != "occlusionTexture"
		   && key != "emissiveTexture" && key != "emissiveFactor"
		   && key != "alphaMode" && key != "alphaCutoff"
		   && key != "doubleSided")
		{
			qWarning() << "gltf::Material key" << key
			           << "parsing not implemented. In this case contains:"
			           << json[key];
		}
	}
	name = json["name"].toString();
	pbrMetallicRoughness.load(json["pbrMetallicRoughness"].toObject(),
	                          globalTextures);

	for(auto const& key : json["normalTexture"].toObject().keys())
	{
		if(key != "index")
		{
			qWarning() << "gltf::Material::normalTexture key" << key
			           << "parsing not implemented. In this case contains:"
			           << json[key];
		}
	}
	int idx = json["normalTexture"].toObject()["index"].toInt(-1);
	if(idx >= 0)
	{
		normalTexture = &globalTextures[idx];
	}

	for(auto const& key : json["occlusionTexture"].toObject().keys())
	{
		if(key != "index" && key != "strength")
		{
			qWarning() << "gltf::Material::occlusionTexture key" << key
			           << "parsing not implemented. In this case contains:"
			           << json[key];
		}
	}
	idx = json["occlusionTexture"].toObject()["index"].toInt(-1);
	if(idx >= 0)
	{
		occlusionTexture = &globalTextures[idx];
	}
	occlusionStrength
	    = json["occlusionTexture"].toObject()["strength"].toDouble(1.f);

	for(auto const& key : json["emissiveTexture"].toObject().keys())
	{
		if(key != "index")
		{
			qWarning() << "gltf::Material::emissiveTexture key" << key
			           << "parsing not implemented. In this case contains:"
			           << json[key];
		}
	}
	idx = json["emissiveTexture"].toObject()["index"].toInt(-1);
	if(idx >= 0)
	{
		emissiveTexture = &globalTextures[idx];
	}
	if(json["emissiveFactor"].isArray())
	{
		for(unsigned int i(0); i < 3; ++i)
		{
			emissiveFactor[i] = json["emissiveFactor"].toArray()[i].toDouble();
		}
	}
	const QString alphaModeStr = json["alphaMode"].toString("OPAQUE");
	alphaMode                  = AlphaMode::OPAQUE;
	alphaCutoff                = -1.f;
	if(alphaModeStr == "MASK")
	{
		alphaMode   = AlphaMode::MASK;
		alphaCutoff = json["alphaCutoff"].toDouble(0.5);
	}
	else if(alphaModeStr == "BLEND")
	{
		alphaMode = AlphaMode::BLEND;
	}
	doubleSided = json["doubleSided"].toBool(false);
}

void Mesh::Primitive::load(QJsonObject const& json,
                           std::vector<Accessor> const& globalAccessors,
                           std::vector<Material> const& globalMaterials)
{
	for(auto const& key : json.keys())
	{
		if(key != "attributes" && key != "indices" && key != "material"
		   && key != "mode")
		{
			qWarning() << "gltf::Mesh::Primitive key" << key
			           << "parsing not implemented. In this case contains :"
			           << json[key];
		}
	}

	int idx = json["indices"].toInt(-1);
	if(idx >= 0)
	{
		indices = &globalAccessors[idx];
	}
	idx = json["material"].toInt(-1);
	if(idx >= 0)
	{
		material = &globalMaterials[idx];
	}
	else
	{
		material = &globalMaterials.back();
	}
	mode = modeFromInt(json["mode"].toInt(4));

	for(auto const& key : json["attributes"].toObject().keys())
	{
		if(key != "POSITION" && key != "NORMAL" && key != "TEXCOORD_0"
		   && key != "COLOR_0" && key != "TANGENT")
		{
			qWarning() << "gltf::Mesh::Primitive::attributes key" << key
			           << "parsing not implemented. In this case contains:"
			           << json[key];
		}

		const int idx = json["attributes"].toObject()[key].toInt(-1);
		if(idx >= 0)
		{
			attributes[key] = &globalAccessors[idx];
		}
	}
}

PrimitiveType Mesh::Primitive::modeFromInt(int mode)
{
	switch(mode)
	{
		case 0:
			return PrimitiveType::POINTS;
		case 1:
			return PrimitiveType::LINES;
		case 2:
			return PrimitiveType::LINE_LOOP;
		case 3:
			return PrimitiveType::LINE_STRIP;
		case 4:
			return PrimitiveType::TRIANGLES;
		case 5:
			return PrimitiveType::TRIANGLE_STRIP;
		case 6:
			return PrimitiveType::TRIANGLE_FAN;
		default:
			break;
	}
	qWarning() << "Unknown mode (or primitive type)" << mode;
	return PrimitiveType::TRIANGLES;
}

void Mesh::load(QJsonObject const& json,
                std::vector<Accessor> const& globalAccessors,
                std::vector<Material> const& globalMaterials)
{
	for(auto const& key : json.keys())
	{
		if(key != "name" && key != "primitives")
		{
			qWarning() << "gltf::Mesh key" << key
			           << "parsing not implemented. In this case contains:"
			           << json[key];
		}
	}
	name = json["name"].toString();
	for(auto const& primitiveJSON : json["primitives"].toArray())
	{
		primitives.emplace_back();
		primitives.back().load(primitiveJSON.toObject(), globalAccessors,
		                       globalMaterials);
	}
}

void Node::load(QJsonObject const& json, std::vector<Mesh> const& globalMeshes)
{
	for(auto const& key : json.keys())
	{
		if(key != "name" && key != "mesh" && key != "translation"
		   && key != "matrix" && key != "children" && key != "rotation"
		   && key != "scale")
		{
			qWarning() << "gltf::GLTFNode key" << key
			           << "parsing not implemented. In this case contains:"
			           << json[key];
		}
	}
	for(auto const& childJSON : json["children"].toArray())
	{
		int nodeIdx = childJSON.toInt(-1);
		if(nodeIdx >= 0)
		{
			childrenIds.push_back(nodeIdx);
		}
	}
	name              = json["name"].toString();
	const int meshIdx = json["mesh"].toInt(-1);
	if(meshIdx >= 0)
	{
		mesh = &globalMeshes[meshIdx];
	}
	if(json["matrix"].isArray())
	{
		for(unsigned int i(0); i < 4; ++i)
		{
			QVector4D column;
			for(unsigned int j(0); j < 4; ++j)
			{
				column[j] = json["matrix"].toArray()[i * 4 + j].toInt();
				matrix.setColumn(i, column);
			}
		}
	}
	else
	{
		QVector3D translation;
		int i = 0;
		for(auto const& transVal : json["translation"].toArray())
		{
			translation[i] = transVal.toDouble();
			++i;
		}
		matrix.translate(translation);
		QVector4D rotation{0.f, 0.f, 0.f, 1.f};
		i = 0;
		for(auto const& rotVal : json["rotation"].toArray())
		{
			rotation[i] = rotVal.toDouble();
			++i;
		}
		matrix.rotate(QQuaternion{rotation});
		QVector3D scale{1.f, 1.f, 1.f};
		i = 0;
		for(auto const& scaleVal : json["scale"].toArray())
		{
			scale[i] = scaleVal.toDouble();
			++i;
		}
		matrix.scale(scale);
	}
}

void Node::setChildren(std::vector<Node> const& globalNodes)
{
	for(auto id : childrenIds)
	{
		children.emplace_back(&globalNodes[id]);
	}
}

void Scene::load(QJsonObject const& json, std::vector<Node> const& globalNodes)
{
	for(auto const& key : json.keys())
	{
		if(key != "name" && key != "nodes")
		{
			qWarning() << "gltf::Scene key" << key
			           << "parsing not implemented. In this case contains:"
			           << json[key];
		}
	}
	name = json["name"].toString();
	for(auto const& nodeIdxJSON : json["nodes"].toArray())
	{
		nodes.push_back(&globalNodes[nodeIdxJSON.toInt(-1)]);
	}
}

void Asset::load(QJsonObject const& json)
{
	for(auto const& key : json.keys())
	{
		if(key != "copyright" && key != "generator" && key != "version"
		   && key != "minVersion")
		{
			qWarning() << "gltf::Asset key" << key
			           << "parsing not implemented. In this case contains:"
			           << json[key];
		}
	}
	copyright  = json["copyright"].toString();
	generator  = json["generator"].toString();
	version    = json["version"].toString();
	minVersion = json["minVersion"].toString();
}

bool CPUData::load(QJsonObject const& json,
                   std::vector<char>&& glbBinBufferChunk)
{
	for(auto const& key : json.keys())
	{
		if(key != "asset" && key != "scene" && key != "scenes" && key != "nodes"
		   && key != "meshes" && key != "accessors" && key != "bufferViews"
		   && key != "buffers" && key != "materials" && key != "images"
		   && key != "samplers" && key != "textures")
		{
			qWarning() << "GLTF key" << key
			           << "parsing not implemented. In this case contains:"
			           << json[key];
		}
	}

	asset.load(json["asset"].toObject());
	auto version = asset.version;
	if(version[0] != '2' || version[1] != '.')
	{
		qWarning() << "Attempting loading GLTF" << version
		           << "Only GLTF 2.x is supported. Aborted loading...";
		return false;
	}

	for(auto const& bufferJSON : json["buffers"].toArray())
	{
		buffers.emplace_back(bufferJSON.toObject(), glbBinBufferChunk);
	}
	// add default empty buffer for undefined indices
	buffers.emplace_back();

	for(auto const& bufferViewJSON : json["bufferViews"].toArray())
	{
		bufferViews.emplace_back(bufferViewJSON.toObject(), buffers);
	}

	for(auto const& accessorJSON : json["accessors"].toArray())
	{
		accessors.emplace_back();
		accessors.back().load(accessorJSON.toObject(), bufferViews);
	}

	for(auto const& samplerJSON : json["samplers"].toArray())
	{
		samplers.emplace_back(samplerJSON.toObject());
	}
	// add default sampler for undefined sampler
	samplers.emplace_back(QJsonObject{});

	for(auto const& imageJSON : json["images"].toArray())
	{
		images.emplace_back(imageJSON.toObject(), bufferViews);
	}

	for(auto const& textureJSON : json["textures"].toArray())
	{
		textures.emplace_back();
		textures.back().load(textureJSON.toObject(), samplers, images);
	}

	for(auto const& materialJSON : json["materials"].toArray())
	{
		materials.emplace_back();
		materials.back().load(materialJSON.toObject(), textures);
	}
	materials.emplace_back(); // add a default material

	for(auto const& meshJSON : json["meshes"].toArray())
	{
		meshes.emplace_back();
		meshes.back().load(meshJSON.toObject(), accessors, materials);
	}

	for(auto const& nodeJSON : json["nodes"].toArray())
	{
		nodes.emplace_back();
		nodes.back().load(nodeJSON.toObject(), meshes);
	}
	for(auto& node : nodes)
	{
		node.setChildren(nodes);
	}

	for(auto const& sceneJSON : json["scenes"].toArray())
	{
		scenes.emplace_back();
		scenes.back().load(sceneJSON.toObject(), nodes);
	}

	if(!scenes.empty())
	{
		const int sceneIdx = json["scene"].toInt(-1);
		if(sceneIdx >= 0)
		{
			scene = &scenes[sceneIdx];
		}
	}

	return true;
}

} // namespace gltf
