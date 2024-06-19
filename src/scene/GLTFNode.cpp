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

#include "scene/GLTFNode.hpp"

#include <QJsonDocument>
#include <QJsonObject>

GLTFNode::GLTFNode(QString src, std::map<QString, Node*>& nodesDict)
    : Node(nodesDict, src.split("/").last())
    , src(std::move(src))
{
	loadCPU();
	loadGPU();

	BoundingSphere bs{{}, 0.f};
	for(auto const& node : gpuData->nodes)
	{
		auto nodeBS = node.getBoundingSphere();
		if(bs.radius == 0.f)
		{
			bs = nodeBS;
		}
		else
		{
			bs = bs.merged(nodeBS);
		}
	}
	boundingSphere = bs;
}

void GLTFNode::loadCPU()
{
	gltfModel.setColumn(0, QVector4D(0.f, 1.f, 0.f, 0.f));
	gltfModel.setColumn(1, QVector4D(0.f, 0.f, 1.f, 0.f));
	gltfModel.setColumn(2, QVector4D(1.f, 0.f, 0.f, 0.f));

	// FIRST PARSE ALL FILE AND STORE ITS CONTENT IN RAM
	QString srcDir(QFileInfo(src).absoluteDir().absolutePath());
	QString currentDir(QDir::current().absolutePath());

	QJsonObject json;
	{
		QFile file(src);
		json = QJsonDocument::fromJson(file.open(QIODevice::ReadOnly)
		                                   ? file.readAll()
		                                   : QByteArray())
		           .object();
	}

	// metadata
	qDebug() << "Loading" << src;
	QDir::setCurrent(srcDir);
	if(!cpuData.load(json))
	{
		QDir::setCurrent(currentDir);
		return;
	}
	QDir::setCurrent(currentDir);
}

void GLTFNode::loadGPU()
{
	gpuData = std::make_unique<gltf::GPUData>(cpuData);
}

std::vector<std::pair<GLMesh const&, QMatrix4x4>>
    GLTFNode::getShadowCastingMeshes() const
{
	std::vector<std::pair<GLMesh const&, QMatrix4x4>> result;
	for(auto const& node : gpuData->nodes)
	{
		for(auto const& pair : node.getMeshes(getModel()))
		{
			result.push_back(pair);
		}
	}
	return result;
}

bool GLTFNode::setTransform(QString const& subNodeName,
                            QMatrix4x4 const& transform)
{
	if(gpuData->nodesDict.count(subNodeName) > 0)
	{
		gpuData->nodesDict[subNodeName]->transform = transform;
		return true;
	}
	return false;
}

QMatrix4x4 GLTFNode::preMultiplyTransform() const
{
	return gltfModel;
}

void GLTFNode::doRender(BasicCamera const& cam,
                        std::vector<Light const*> const& lights,
                        GLTexture const& brdfLUT, bool /*environment*/)
{
	for(auto const& gpuNode : gpuData->nodes)
	{
		gpuNode.render(cam, lights, getModel(), getIrradianceMap(),
		               getPrefilteredMap(), brdfLUT);
	}
}
