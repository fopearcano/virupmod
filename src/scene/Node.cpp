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

#include "scene/Node.hpp"

#include "Timings.hpp"
#include "camera/BasicCamera.hpp"
#include "scene/Scene.hpp"

Node::Node(std::map<QString, Node*>& nodesDict, QString name)
    : nodesDict(nodesDict)
    , name(std::move(name))
    , irradiancemap(GLTexture::TexCubemapProperties(16, GL_RGBA32F))
    , prefilteredmap(GLTexture::TexCubemapProperties(256, GL_RGBA32F))
// , bsShader("default")
{
	if(!this->name.isEmpty())
	{
		if(nodesDict.count(this->name) > 0)
		{
			qCritical() << this->name << "is duplicated as Node name.";
		}
		else
		{
			nodesDict[this->name] = this;
		}
	}
	prefilteredmap.generateMipmap();

	irradiancemap.setName("irradiance - " + this->name);
	prefilteredmap.setName("prefilteredmap - " + this->name);

	/* bsShader.setUniform("color", QVector3D(255, 0, 0));
	bsShader.setUniform("alpha", 0.1f);
	Primitives::setAsUnitSphere(bsMesh, bsShader);*/
}

Node::Node(Node&& moved) noexcept
    : boundingSphere(std::move(moved.boundingSphere))
    , nodesDict(moved.nodesDict)
    , name(std::move(moved.name))
    , model(moved.model)
    , transformedBoundingSphere(std::move(moved.transformedBoundingSphere))
    , irradiancemap(std::move(moved.irradiancemap))
    , prefilteredmap(std::move(moved.prefilteredmap))
    , envIsRendering(moved.envIsRendering)
    , visibility(moved.visibility)
    , computedEnvOnce(moved.computedEnvOnce)
// , bsShader(std::move(moved.bsShader))
// , bsMesh(std::move(moved.bsMesh))
{
	if(!this->name.isEmpty())
	{
		nodesDict[this->name] = this;
	}
}

void Node::setModel(QMatrix4x4 model)
{
	this->model               = model;
	transformedBoundingSphere = boundingSphere.transformed(model);
}

float Node::computeVisibility(BasicCamera const& camera)
{
	if(camera.shouldBeCulled(transformedBoundingSphere))
	{
		visibility = 0.f;
		return visibility;
	}
	visibility = atan2(
	    transformedBoundingSphere.radius,
	    (transformedBoundingSphere.position - camera.getWorldSpacePosition())
	        .length());
	return visibility;
}

void Node::computeEnvMap(BasicCamera& camera, GLFramebufferObject const& envmap,
                         Scene& scene)
{
	GLStateSet glState({{GL_TEXTURE_CUBE_MAP_SEAMLESS, true}});
	computedEnvOnce = true;

	envIsRendering = true;
	// render envmap
	auto renderFunc
	    = [&camera, &envmap, &scene](bool /*overrideCamera*/,
	                                 QMatrix4x4 overrView, QMatrix4x4 overrProj)
	{
		QMatrix4x4 viewBack(camera.getView()), projBack(camera.getProj());
		auto windowSizeBack(camera.getWindowSize());

		auto renderSize(envmap.getSize());
		camera.setWindowSize(renderSize);
		GLHandler::glf().glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
		                         | GL_STENCIL_BUFFER_BIT);
		camera.setProj(overrProj);
		camera.setView(overrView);
		camera.update2D({});
		camera.uploadMatrices();
		scene.render(camera, true);

		camera.setProj(projBack);
		camera.setView(viewBack);
		camera.setWindowSize(windowSizeBack);
	};

	GLHandler::generateEnvironmentMap(envmap, renderFunc,
	                                  transformedBoundingSphere.position);
	envmap.getColorAttachmentTexture().generateMipmap();

	envIsRendering = false;

	/*//CLASSIC
	GLComputeShader generateirradiancemap("gi/generateirradiancemap");
	Timings::start("generateirradiancemap");
	auto size(irradiancemap.getSize());
	generateirradiancemap.exec(
	    {{envmap.getColorAttachmentTexture(), GLComputeShader::SAMPLER},
	     {irradiancemap, GLComputeShader::W}},
	    {static_cast<unsigned int>(size[0]), static_cast<unsigned int>(size[1]),
	     6});
	irradiancemap.generateMipmap();
	Timings::end("generateirradiancemap");
	// END CLASSIC*/

	// SH
	auto size = envmap.getColorAttachmentTexture().getSize();
	Timings::start("SH");
	GLComputeShader sh("gi/sphericalharmonics");
	GLBuffer shCoeffs(GL_SHADER_STORAGE_BUFFER);
	std::vector<float> d(9 * 4);
	shCoeffs.setData(d, GL_DYNAMIC_DRAW);
	shCoeffs.bindBase(1);
	// with some luck using a lower resolution envmap won't affect the result
	// too much
	sh.exec({{envmap.getColorAttachmentTexture(), GLComputeShader::R, 1}},
	        {static_cast<unsigned int>(size[0]) / 2,
	         static_cast<unsigned int>(size[1]) / 2, 6});
	Timings::end("SH");

	GLComputeShader irrfromSH("gi/irradiancefromSH");
	Timings::start("irradiancefromSH");
	size = irradiancemap.getSize();
	irrfromSH.exec({{irradiancemap, GLComputeShader::W}},
	               {static_cast<unsigned int>(size[0]),
	                static_cast<unsigned int>(size[1]), 6});
	irradiancemap.generateMipmap();
	Timings::end("irradiancefromSH");
	// END SH*/

	GLComputeShader prefilter("gi/prefilter");
	Timings::start("prefilter");
	size = prefilteredmap.getSize();
	for(int i(0); i < 5; ++i)
	{
		unsigned int divisor(1u << static_cast<unsigned int>(i));
		prefilter.setUniform("roughness", i / 4.f);
		prefilter.exec(
		    {{envmap.getColorAttachmentTexture(), GLComputeShader::SAMPLER},
		     {prefilteredmap, GLComputeShader::W, i}},
		    {static_cast<unsigned int>(size[0] / divisor),
		     static_cast<unsigned int>(size[1] / divisor), 6});
	}
	Timings::end("prefilter");
}

void Node::render(BasicCamera const& cam,
                  std::vector<Light const*> const& lights,
                  GLTexture const& brdfLUT, bool environment)
{
	if(visibility == 0.f && !environment)
	{
		return;
	}

	if(envIsRendering || (isDirectLightSource() && environment))
	{
		return;
	}

	doRender(cam, lights, brdfLUT, environment);
}

void Node::renderTransparent(BasicCamera const& cam,
                             std::vector<Light const*> const& lights,
                             GLTexture const& brdfLUT, bool environment)
{
	if(visibility == 0.f && !environment)
	{
		return;
	}

	if(envIsRendering || (isDirectLightSource() && environment))
	{
		return;
	}

	doRenderTransparent(cam, lights, brdfLUT, environment);

	/*if(showBoundingSphere)
	{
	    GLStateSet glState({{GL_CULL_FACE, false}});
	    GLBlendSet glBlend(GLBlendSet::BlendState{});
	    QMatrix4x4 bsModel;
	    bsModel.translate(transformedBoundingSphere.position);
	    bsModel.scale(transformedBoundingSphere.radius);
	    GLHandler::setUpRender(bsShader, bsModel);
	    bsMesh.render();
	}*/
}
