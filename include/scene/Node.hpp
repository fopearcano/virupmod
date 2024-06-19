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

#ifndef NODE_HPP
#define NODE_HPP

#include <QMatrix4x4>

#include "camera/BoundingVolumes.hpp"
#include "gl/GLHandler.hpp"

class BasicCamera;
class Scene;
class VRHandler;
class Light;

class Node
{
  public:
	Node(std::map<QString, Node*>& nodesDict, QString name);
	Node(Node const& other)            = delete;
	Node& operator=(Node const& other) = delete;
	Node(Node&& moved) noexcept;
	Node& operator=(Node&& moved) = delete;

	virtual std::vector<std::pair<GLMesh const&, QMatrix4x4>>
	    getShadowCastingMeshes() const = 0;
	virtual bool setTransform(QString const& /*subNodeName*/,
	                          QMatrix4x4 const& /*transform*/)
	{
		return false;
	};
	BoundingSphere getBoundingSphere() const { return boundingSphere; };
	virtual bool usesGlobalIllumination() const { return true; };
	virtual bool isDirectLightSource() const { return false; };
	void setModel(QMatrix4x4 model);
	void addChild(std::unique_ptr<Node>&& child);
	void eraseChild(unsigned int id);
	void eraseChild(QString const& name);
	bool hasComputedEnvOnce() const { return computedEnvOnce; };
	float computeVisibility(BasicCamera const& camera);
	void computeEnvMap(BasicCamera& camera, GLFramebufferObject const& envmap,
	                   Scene& scene);
	void render(BasicCamera const& cam, std::vector<Light const*> const& lights,
	            GLTexture const& brdfLUT, bool environment = false);
	void renderTransparent(BasicCamera const& cam,
	                       std::vector<Light const*> const& lights,
	                       GLTexture const& brdfLUT, bool environment = false);
	virtual ~Node();

  protected:
	QMatrix4x4 getModel() const { return model * preMultiplyTransform(); };
	GLTexture const& getIrradianceMap() const { return irradiancemap; };
	GLTexture const& getPrefilteredMap() const { return prefilteredmap; };
	virtual QMatrix4x4 preMultiplyTransform() const { return {}; };
	virtual void doRender(BasicCamera const& cam,
	                      std::vector<Light const*> const& lights,
	                      GLTexture const& brdfLUT, bool environment)
	    = 0;
	virtual void doRenderTransparent(
	    BasicCamera const& /*cam*/, std::vector<Light const*> const& /*lights*/,
	    GLTexture const& /*brdfLUT*/, bool /*environment*/){};

	BoundingSphere boundingSphere;
	Node* getChild(unsigned int id);
	Node* getChild(QString const& name);

	std::map<QString, Node*>& nodesDict;

  private:
	std::vector<std::unique_ptr<Node>> children;

	QString name;
	QMatrix4x4 model;
	BoundingSphere transformedBoundingSphere;

	GLTexture irradiancemap;
	GLTexture prefilteredmap;

	bool envIsRendering  = false;
	float visibility     = 0.f;
	bool computedEnvOnce = false;

	// DEBUG
	/*bool showBoundingSphere = true;
	GLShaderProgram bsShader;
	GLMesh bsMesh;*/
};

#endif // NODE_HPP
