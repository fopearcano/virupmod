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

#ifndef GLTFCPUDATA_HPP
#define GLTFCPUDATA_HPP

#include "gl/GLHandler.hpp"

namespace gltf
{
struct Asset
{
	QString copyright;
	QString generator;
	QString version;
	QString minVersion;
	void load(QJsonObject const& json);
};
struct Buffer
{
	Buffer(QJsonObject const& json);
	QString name;
	std::vector<char> data;
};
struct BufferView
{
	enum Target
	{
		ARRAY_BUFFER         = 34962,
		ELEMENT_ARRAY_BUFFER = 34963,
	};

	BufferView(QJsonObject const& json,
	           std::vector<Buffer> const& globalBuffers);
	Buffer const& buffer;
	int byteOffset = 0;
	int byteLength = 0;
	int byteStride = 0;
	Target target;
	QString name;
	Buffer const& loadBuffer(QJsonObject const& json,
	                         std::vector<Buffer> const& globalBuffers);
};
struct Accessor
{
	enum ComponentType
	{
		BYTE           = 5120,
		UNSIGNED_BYTE  = 5121,
		SHORT          = 5122,
		UNSIGNED_SHORT = 5123,
		UNSIGNED_INT   = 5125,
		FLOAT          = 5126,
	};

	BufferView const* bufferView = nullptr;
	int byteOffset;
	ComponentType componentType;
	int count;
	QString type;
	std::vector<double> max; // have enough to store a large int
	std::vector<double> min; // have enough to store a large int
	QString name;
	void load(QJsonObject const& json,
	          std::vector<BufferView> const& globalBufferViews);
	int sizeInBytes(ComponentType type);
	int typeDimensions() const;
};
struct Sampler
{
	Sampler(QJsonObject const& json);
	GLTexture::Sampler glSampler;
	QString name;
	static GLint convertFilter(int filter);
	static GLint convertWrap(int wrap);
};
struct Image
{
	Image(QJsonObject const& json);
	QImage data;
	QString name;
};
struct Texture
{
	Sampler const* sampler = nullptr;
	Image const* source    = nullptr;
	void load(QJsonObject const& json,
	          std::vector<Sampler> const& globalSamplers,
	          std::vector<Image> const& globalImages);
};
struct Material
{
	struct PBRMetallicRoughness
	{
		QColor baseColorFactor                  = QColor(255, 0, 255);
		Texture const* baseColorTexture         = nullptr;
		float metallicFactor                    = 1.f;
		float roughnessFactor                   = 1.f;
		Texture const* metallicRoughnessTexture = nullptr;
		void load(QJsonObject const& json,
		          std::vector<Texture> const& globalTextures);
	};
	QString name;
	PBRMetallicRoughness pbrMetallicRoughness;
	Texture const* normalTexture    = nullptr;
	Texture const* occlusionTexture = nullptr;
	float occlusionStrength         = 1.f;
	Texture const* emissiveTexture  = nullptr;
	QVector3D emissiveFactor;
	bool doubleSided = false;
	void load(QJsonObject const& json,
	          std::vector<Texture> const& globalTextures);
};
struct Mesh
{
	struct Primitive
	{
		std::map<QString, Accessor const*> attributes;
		Accessor const* indices  = nullptr;
		Material const* material = nullptr;
		PrimitiveType mode       = PrimitiveType::TRIANGLES;
		void load(QJsonObject const& json,
		          std::vector<Accessor> const& globalAccessors,
		          std::vector<Material> const& globalMaterials);
		PrimitiveType modeFromInt(int mode);
	};

	QString name;
	std::vector<Primitive> primitives;
	void load(QJsonObject const& json,
	          std::vector<Accessor> const& globalAccessors,
	          std::vector<Material> const& globalMaterials);
};
struct Node
{
	std::vector<unsigned int> childrenIds;
	std::vector<Node const*> children;
	QString name;
	Mesh const* mesh = nullptr;
	QMatrix4x4 matrix;
	void load(QJsonObject const& json, std::vector<Mesh> const& globalMeshes);
	void setChildren(std::vector<Node> const& globalNodes);
};
struct Scene
{
	QString name;
	std::vector<Node const*> nodes;
	void load(QJsonObject const& json, std::vector<Node> const& globalNodes);
};

struct CPUData
{
	Asset asset;
	std::vector<Buffer> buffers;
	std::vector<BufferView> bufferViews;
	std::vector<Accessor> accessors;
	std::vector<Sampler> samplers;
	std::vector<Image> images;
	std::vector<Texture> textures;
	std::vector<Material> materials;
	std::vector<Mesh> meshes;
	std::vector<Node> nodes;
	std::vector<Scene> scenes;
	Scene const* scene = nullptr;

	bool load(QJsonObject const& json);
};

} // namespace gltf

#endif // GLTFCPUDATA_HPP
