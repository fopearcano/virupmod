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

#ifndef HVR_SCENE_HPP
#define HVR_SCENE_HPP

#include "Light.hpp"
#include "memory.hpp"
#include "scene/Node.hpp"
#include <QElapsedTimer>
#include <random>

class BasicCamera;

namespace hvr
{
class Scene
{
  public:
	Scene();
	void addNode(std::unique_ptr<Node>&& newNode);
	void addLight(std::unique_ptr<Light>&& newLight);
	void update(BasicCamera& camera);
	void render(BasicCamera const& cam, bool environment = false);
	virtual ~Scene() = default;

  protected:
	virtual std::vector<Node*> sortedNodes() const;
	virtual void updateNodes(BasicCamera& /*camera*/) {};
	std::map<QString, Node*> nodesDict;
	std::vector<std::unique_ptr<Light>> lights;

	bool renderLights = false;

	std::vector<std::unique_ptr<Node>> rootNodes;

  private:
	GLFramebufferObject envmap;
	GLTexture brdfLUT;

	std::mt19937_64 generator;
	std::uniform_real_distribution<float> distribution;
};
} // namespace hvr

#endif // HVR_SCENE_HPP
