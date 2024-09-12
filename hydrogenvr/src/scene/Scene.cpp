/*
    Copyright (C) 2023 Florian Cabot <florian.cabot@hotmail.fr>

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

#include "scene/Scene.hpp"

#include "camera/BasicCamera.hpp"

#include <ctime>
namespace hvr
{
Scene::Scene()
    : envmap(GLTexture::TexCubemapProperties(512, GL_RGBA32F))
    , brdfLUT(GLTexture::Tex2DProperties(512, 512, GL_RGBA32F))
    // NOLINTNEXTLINE(cert-msc32-c,cert-msc51-cpp)
    , generator(static_cast<unsigned int>(std::time(nullptr)))
    , distribution(0.0, 1.0)
{
	const GLComputeShader brdf_lut("gi/brdf_lut");
	brdf_lut.exec({{brdfLUT, GLComputeShader::W}}, {512, 512, 1});

	envmap.setColorAttachmentName("envmap");
}

void Scene::addNode(std::unique_ptr<Node>&& newNode)
{
	rootNodes.emplace_back(std::move(newNode));
}

void Scene::addLight(std::unique_ptr<Light>&& newLight)
{
	lights.emplace_back(std::move(newLight));
}

void Scene::update(BasicCamera& camera)
{
	updateNodes(camera);

	std::vector<GLMesh const*> shadowCastMeshes;
	std::vector<QMatrix4x4> shadowCastModels;
	for(auto const& pair : nodesDict)
	{
		auto* node = pair.second;
		for(auto const& pair : node->getShadowCastingMeshes())
		{
			shadowCastMeshes.push_back(&pair.first);
			shadowCastModels.push_back(pair.second);
		}
	}
	for(auto const& light : lights)
	{
		light->generateShadowMap(shadowCastMeshes, shadowCastModels);
	}

	// determine which node will get updated for this frame
	float sum = 0.f;
	std::vector<std::pair<Node*, float>> visibilities;
	for(auto const& pair : nodesDict)
	{
		auto* node = pair.second;
		auto v     = node->computeVisibility(camera);
		if(v <= 0.f || !node->usesGlobalIllumination())
		{
			continue;
		}
		sum += v;
		visibilities.emplace_back(node, v);
	}

	// if there is anything to update
	if(!visibilities.empty())
	{
		Node* nodeToUpdate(visibilities.back().first);
		for(unsigned int i(0); i < visibilities.size() - 1; ++i)
		{
			auto const& pair = visibilities[i];
			// pick if passes random test between all nodes
			// or if has never computed its env map
			if(!pair.first->hasComputedEnvOnce()
			   || distribution(generator) <= pair.second / sum)
			{
				nodeToUpdate = pair.first;
				break;
			}
			sum -= pair.second;
		}
		nodeToUpdate->computeEnvMap(camera, envmap, *this);
	}
}

void Scene::render(BasicCamera const& cam, bool environment)
{
	if(!environment && renderLights)
	{
		for(auto const& light : lights)
		{
			light->render(0.542f * M_PI / 180.f);
		}
	}

	std::vector<Light const*> lightsPtr;
	lightsPtr.reserve(lights.size());
	for(auto const& light : lights)
	{
		lightsPtr.push_back(light.get());
	}

	auto sorted(sortedNodes());
	for(auto const& node : sorted)
	{
		node->render(cam, lightsPtr, brdfLUT, environment);
	}

	auto it(sorted.end());
	while(it != sorted.begin())
	{
		--it;
		(*it)->renderTransparent(cam, lightsPtr, brdfLUT, environment);
	}
}

std::vector<Node*> Scene::sortedNodes() const
{
	std::vector<Node*> result;
	result.reserve(rootNodes.size());
	for(auto const& node : rootNodes)
	{
		result.push_back(node.get());
	}
	return result;
}
} // namespace hvr
