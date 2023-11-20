/*
    Copyright (C) 2020 Florian Cabot <florian.cabot@hotmail.fr>

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

#include "Renderer.hpp"

#include "AbstractMainWin.hpp"

Renderer::Renderer(AbstractMainWin& window, VRHandler& vrHandler)
    : window(window)
    , vrHandler(vrHandler)
{
}

void Renderer::init(Dialog3DWheel& dialog3dWheel)
{
	if(initialized)
	{
		clean();
	}

	this->dialog3dWheel = &dialog3dWheel;

	QObject::connect(&vrHandler, &VRHandler::renderTargetSizeChanged,
	                 [this]() { this->updateRenderTargets(); });

	dbgCamera = std::make_unique<DebugCamera>(vrHandler);
	dbgCamera->lookAt({2, 0, 2}, {0, 0, 0}, {0, 0, 1});

	auto defaultCam = std::make_unique<BasicCamera>(vrHandler);
	defaultCam->lookAt({1, 1, 1}, {0, 0, 0}, {0, 0, 1});
	appendSceneRenderPath("default", RenderPath(std::move(defaultCam)));

	reloadPostProcessingTargets();
	updateFOV();

	initialized = true;
}

void Renderer::updateRenderTargets()
{
	if(!initialized)
	{
		return;
	}

	updateFOV();
	reloadPostProcessingTargets();
}

QSize Renderer::getSize(bool ignoreVR) const
{
	QSize renderSize(window.size().width(), window.size().height());
	renderSize *= window.screen()->devicePixelRatio();
	if(vrHandler.isEnabled() && !ignoreVR)
	{
		renderSize = vrHandler.getEyeRenderTargetSize();
	}
	else if(QSettings().value("window/forcerenderresolution").toBool())
	{
		renderSize.setWidth(QSettings().value("window/forcewidth").toInt());
		renderSize.setHeight(QSettings().value("window/forceheight").toInt());
	}
	return renderSize;
}

float Renderer::getRenderTargetAspectRatio() const
{
	QSize renderSize(getSize());
	float aspectRatio(static_cast<float>(renderSize.width())
	                  / static_cast<float>(renderSize.height()));
	return aspectRatio;
}

float Renderer::getAspectRatioFromFOV() const
{
	return tan(M_PI * hFOV / 360.f) / tan(M_PI * vFOV / 360.f);
}

BasicCamera const& Renderer::getCamera(QString const& pathId) const
{
	for(auto const& pair : sceneRenderPipeline)
	{
		if(pair.first == pathId)
		{
			return *(pair.second.camera);
		}
	}
	throw(std::domain_error(std::string("Path id doesn't exist.")
	                        + pathId.toStdString()));
}

BasicCamera& Renderer::getCamera(QString const& pathId)
{
	for(auto const& pair : sceneRenderPipeline)
	{
		if(pair.first == pathId)
		{
			return *(pair.second.camera);
		}
	}
	throw(std::domain_error(std::string("Path id doesn't exist.")
	                        + pathId.toStdString()));
}

QImage Renderer::getLastFrame() const
{
	return mainRenderTarget->postProcessingTargets
	    .at(postProcessingPipeline_.size() % 2)
	    .copyColorBufferToQImage()
	    .mirrored(false, true);
}

void Renderer::appendSceneRenderPath(QString const& id, RenderPath path)
{
	sceneRenderPipeline_.emplace_back(id, std::move(path));
}

void Renderer::insertSceneRenderPath(QString const& id, RenderPath path,
                                     unsigned int pos)
{
	sceneRenderPipeline_.emplace(
	    std::next(sceneRenderPipeline_.begin(), pos),
	    std::pair<QString, RenderPath>(id, std::move(path)));
}

void Renderer::removeSceneRenderPath(QString const& id)
{
	for(auto it(sceneRenderPipeline_.begin()); it != sceneRenderPipeline_.end();
	    ++it)
	{
		if(it->first == id)
		{
			sceneRenderPipeline_.erase(it);
			break;
		}
	}
}

void Renderer::appendPostProcessingShader(QString const& id,
                                          QString const& computeName,
                                          QMap<QString, QString> const& defines)
{
	postProcessingPipeline_.emplace_back(
	    std::make_pair(id, GLComputeShader(computeName, defines)));
}

void Renderer::insertPostProcessingShader(QString const& id,
                                          QString const& computeName,
                                          unsigned int pos)
{
	postProcessingPipeline_.emplace(
	    std::next(postProcessingPipeline_.begin(), pos),
	    std::make_pair(id, GLComputeShader(computeName)));
}

void Renderer::removePostProcessingShader(QString const& id)
{
	for(auto it(postProcessingPipeline_.begin());
	    it != postProcessingPipeline_.end(); ++it)
	{
		if(it->first == id)
		{
			postProcessingPipeline_.erase(it);
			break;
		}
	}
}

void Renderer::reloadPostProcessingTargets()
{
	QSize newSize(getSize());
	unsigned int samples(
	    static_cast<unsigned int>(1)
	    << QSettings().value("graphics/antialiasing").toUInt());

	mainRenderTarget = std::make_unique<MainRenderTarget>(
	    newSize.width(), newSize.height(), samples, projection);
}

void Renderer::updateFOV()
{
	vFOV = QSettings().value("graphics/vfov").toDouble();
	hFOV = QSettings().value("graphics/hfov").toDouble();
	if(vFOV == 0.0)
	{
		if(hFOV == 0.0)
		{
			vFOV = 70.0;
		}
		else
		{
			float a(getRenderTargetAspectRatio());
			vFOV = 360.f * atan(tan(hFOV * M_PI / 360.f) / a) / M_PI;
		}
	}
	if(hFOV == 0.0)
	{
		float a(getRenderTargetAspectRatio());
		hFOV = 360.f * atan(tan(vFOV * M_PI / 360.f) * a) / M_PI;
	}

	for(auto const& pair : sceneRenderPipeline_)
	{
		pair.second.camera->setPerspectiveProj(vFOV, getAspectRatioFromFOV());
	}
	dbgCamera->setPerspectiveProj(vFOV, getAspectRatioFromFOV());

	CalibrationCompass::serverHorizontalFOV()     = getHorizontalFOV();
	CalibrationCompass::serverRenderTargetWidth() = getSize().width();
}

void Renderer::toggleCalibrationCompass()
{
	if(!renderCompass)
	{
		compass       = std::make_unique<CalibrationCompass>();
		renderCompass = true;
	}
	else
	{
		compass.reset();
		renderCompass = false;
	}
}

void Renderer::renderVRControls() const
{
	if(vrHandler.isEnabled())
	{
		vrHandler.renderControllers();
		vrHandler.renderHands();
		dialog3dWheel->render();
	}
}

void Renderer::vrRenderSinglePath(RenderPath& renderPath, QString const& pathId,
                                  bool debug, bool debugInHeadset)
{
	GLHandler::glf().glClear(renderPath.clearMask);
	renderPath.camera->update(angleShiftMat);
	dbgCamera->update(angleShiftMat);

	if(debug && debugInHeadset)
	{
		dbgCamera->uploadMatrices();
	}
	else
	{
		renderPath.camera->uploadMatrices();
	}
	if(pathIdRenderingControllers == pathId && renderControllersBeforeScene)
	{
		renderVRControls();
	}
	// render scene
	if(wireframe)
	{
		GLHandler::beginWireframe();
	}
	window.renderScene(*renderPath.camera, pathId);
	if(pathIdRenderingControllers == pathId && !renderControllersBeforeScene)
	{
		renderVRControls();
	}
	PythonQtHandler::evalScript(
	    "if \"renderScene\" in dir():\n\trenderScene()");
	if(debug && debugInHeadset)
	{
		dbgCamera->renderCamera(*renderPath.camera);
	}
	if(renderCompass)
	{
		compass->render(angleShiftMat);
	}
	if(wireframe)
	{
		GLHandler::endWireframe();
	}
}

void Renderer::vrRender(Side side, bool debug, bool debugInHeadset,
                        bool displayOnScreen)
{
	vrHandler.prepareRendering(side);
	GLHandler::beginRendering(mainRenderTarget->sceneTarget);

	GLStateSet glState({{GL_STENCIL_TEST, true}});
	GLHandler::glf().glClearStencil(0x0);
	GLHandler::glf().glStencilMask(0xFF);
	GLHandler::glf().glStencilFunc(GL_ALWAYS, 1, 0xFF);
	GLHandler::glf().glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	GLHandler::glf().glClear(static_cast<GLuint>(GL_STENCIL_BUFFER_BIT));
	vrHandler.renderHiddenAreaMesh(side);
	GLHandler::glf().glStencilMask(0x00);
	GLHandler::glf().glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	GLHandler::glf().glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	for(auto& pair : sceneRenderPipeline_)
	{
		pair.second.camera->setWindowSize(getSize());
		vrRenderSinglePath(pair.second, pair.first, debug, debugInHeadset);
	}
	mainRenderTarget->sceneTarget.blitColorBufferTo(
	    mainRenderTarget->postProcessingTargets[0]);

	if(computeAverageLuminance)
	{
		lastFrameAverageLuminance += mainRenderTarget->postProcessingTargets[0]
		                                 .getColorAttachmentTexture()
		                                 .getAverageLuminance();
	}

	// do all postprocesses including last one
	int i(0);
	for(auto it(postProcessingPipeline_.begin());
	    it != postProcessingPipeline_.end(); ++it, ++i)
	{
		window.applyPostProcShaderParams(
		    it->first, it->second,
		    mainRenderTarget->postProcessingTargets.at(i % 2));
		auto texs = window.getPostProcessingUniformTextures(
		    it->first, it->second,
		    mainRenderTarget->postProcessingTargets.at(i % 2));
		GLHandler::postProcess(
		    it->second, mainRenderTarget->postProcessingTargets.at(i % 2),
		    mainRenderTarget->postProcessingTargets.at((i + 1) % 2), texs);
	}

	vrHandler.submitRendering(mainRenderTarget->postProcessingTargets.at(
	    postProcessingPipeline_.size() % 2));

	if(displayOnScreen)
	{
		// blit result on screen
		if(vrHandler.forceLeft || vrHandler.forceRight)
		{
			mainRenderTarget->postProcessingTargets
			    .at(postProcessingPipeline_.size() % 2)
			    .showOnWindow(window);
		}
		else if(side == Side::LEFT)
		{
			mainRenderTarget->postProcessingTargets
			    .at(postProcessingPipeline_.size() % 2)
			    .showOnWindow(window, 0, 0, 0.5f, 1.f);
			if(vrHandler.getStereoMultiplier() == 0.0)
			{
				mainRenderTarget->postProcessingTargets
				    .at(postProcessingPipeline_.size() % 2)
				    .showOnWindow(window, 0.5f, 0.f, 1.f, 1.f);
			}
		}
		else
		{
			mainRenderTarget->postProcessingTargets
			    .at(postProcessingPipeline_.size() % 2)
			    .showOnWindow(window, 0.5f, 0.f, 1.f, 1.f);
		}
	}
}

void Renderer::renderFrame(QMatrix4x4 angleShiftMat)
{
	this->angleShiftMat = angleShiftMat;
	bool debug(dbgCamera->isEnabled());
	bool debugInHeadset(dbgCamera->debugInHeadset());
	bool renderingCamIsDebug(debug
	                         && ((debugInHeadset && vrHandler.isEnabled())
	                             || !vrHandler.isEnabled()));
	bool thirdRender(QSettings().value("vr/thirdrender").toBool());

	// main render logic
	if(vrHandler.isEnabled())
	{
		lastFrameAverageLuminance = 0.f;
		if(!vrHandler.forceRight)
		{
			vrRender(Side::LEFT, debug, debugInHeadset,
			         !thirdRender && (!debug || debugInHeadset));
		}
		if((!vrHandler.forceLeft || vrHandler.forceRight)
		   && vrHandler.getStereoMultiplier() != 0.0)
		{
			vrRender(Side::RIGHT, debug, debugInHeadset,
			         !thirdRender && (!debug || debugInHeadset));
		}
		lastFrameAverageLuminance *= 0.5f;

		if(debug && !debugInHeadset)
		{
			renderingCamIsDebug = true;
		}
	}
	// if no VR or debug not in headset, render 2D
	if((!vrHandler.isEnabled() || thirdRender) || (debug && !debugInHeadset))
	{
		auto renderFunc =
		    [=](bool overrideCamera, QMatrix4x4 overrView, QMatrix4x4 overrProj)
		{
			for(auto const& pair : sceneRenderPipeline_)
			{
				auto renderSize(mainRenderTarget->sceneTarget.getSize());
				pair.second.camera->setWindowSize(renderSize);
				GLHandler::glf().glClear(pair.second.clearMask);
				QMatrix4x4 viewBack(pair.second.camera->getView()),
				    projBack(pair.second.camera->getProj());
				if(overrideCamera)
				{
					pair.second.camera->setProj(overrProj);
					pair.second.camera->setView(overrView * viewBack);
				}
				pair.second.camera->update2D(angleShiftMat);
				dbgCamera->update(angleShiftMat);
				if(renderingCamIsDebug)
				{
					dbgCamera->uploadMatrices();
				}
				else
				{
					pair.second.camera->uploadMatrices();
				}
				// render scene
				if(wireframe)
				{
					GLHandler::beginWireframe();
				}

				window.renderScene(*pair.second.camera, pair.first);
				PythonQtHandler::evalScript(
				    "if \"renderScene\" in dir():\n\trenderScene()");
				if(debug)
				{
					dbgCamera->renderCamera(*pair.second.camera);
				}
				if(renderCompass)
				{
					compass->render(angleShiftMat);
				}
				if(wireframe)
				{
					GLHandler::endWireframe();
				}
				if(overrideCamera)
				{
					pair.second.camera->setProj(projBack);
					pair.second.camera->setView(viewBack);
				}
			}
		};

		if(projection == MainRenderTarget::Projection::DEFAULT)
		{
			GLHandler::beginRendering(mainRenderTarget->sceneTarget);
			renderFunc(false, QMatrix4x4(), QMatrix4x4());
			mainRenderTarget->sceneTarget.blitColorBufferTo(
			    mainRenderTarget->postProcessingTargets[0]);
		}
		else if(projection == MainRenderTarget::Projection::PANORAMA360)
		{
			GLHandler::generateEnvironmentMap(mainRenderTarget->sceneTarget,
			                                  renderFunc);
			mainRenderTarget->sceneTarget.getColorAttachmentTexture()
			    .generateMipmap();

			GLShaderProgram shader("postprocess", "panorama360");
			GLHandler::postProcess(shader, mainRenderTarget->sceneTarget,
			                       mainRenderTarget->postProcessingTargets[0]);
		}
		else if(projection == MainRenderTarget::Projection::VR180L
		        || projection == MainRenderTarget::Projection::VR180R)
		{
			QVector3D shift(projection == MainRenderTarget::Projection::VR180L
			                    ? -0.065
			                    : 0.065,
			                0.0, 0.0);

			GLHandler::generateEnvironmentMap(mainRenderTarget->sceneTarget,
			                                  renderFunc, shift);
			mainRenderTarget->sceneTarget.getColorAttachmentTexture()
			    .generateMipmap();
			GLShaderProgram shader("postprocess", "panorama180");
			GLHandler::postProcess(shader, mainRenderTarget->sceneTarget,
			                       mainRenderTarget->postProcessingTargets[0]);
		}
		else if(projection == MainRenderTarget::Projection::VR180)
		{
			int tgtWidth(
			    mainRenderTarget->postProcessingTargets[0].getSize().width()),
			    tgtHeight(mainRenderTarget->postProcessingTargets[0]
			                  .getSize()
			                  .height());
			QVector3D shift(0.065, 0.0, 0.0);

			GLHandler::generateEnvironmentMap(mainRenderTarget->sceneTarget,
			                                  renderFunc, -shift);
			mainRenderTarget->sceneTarget.getColorAttachmentTexture()
			    .generateMipmap();
			GLShaderProgram shader("postprocess", "panorama180");
			GLHandler::postProcess(shader, mainRenderTarget->sceneTarget,
			                       mainRenderTarget->postProcessingTargets[0]);
			mainRenderTarget->postProcessingTargets[0].blitColorBufferTo(
			    mainRenderTarget->postProcessingTargets[1], 0, 0, tgtWidth,
			    tgtHeight, 0, 0, tgtWidth / 2, tgtHeight);

			GLHandler::generateEnvironmentMap(mainRenderTarget->sceneTarget,
			                                  renderFunc, shift);
			mainRenderTarget->sceneTarget.getColorAttachmentTexture()
			    .generateMipmap();
			GLHandler::postProcess(shader, mainRenderTarget->sceneTarget,
			                       mainRenderTarget->postProcessingTargets[0]);
			mainRenderTarget->postProcessingTargets[0].blitColorBufferTo(
			    mainRenderTarget->postProcessingTargets[1], 0, 0, tgtWidth,
			    tgtHeight, tgtWidth / 2, 0, tgtWidth, tgtHeight);
			mainRenderTarget->postProcessingTargets[1].blitColorBufferTo(
			    mainRenderTarget->postProcessingTargets[0]);
		}
		else if(projection == MainRenderTarget::Projection::DOMEMASTER180)
		{
			GLHandler::generateEnvironmentMap(mainRenderTarget->sceneTarget,
			                                  renderFunc);
			mainRenderTarget->sceneTarget.getColorAttachmentTexture()
			    .generateMipmap();

			GLShaderProgram shader("postprocess", "domemaster180");
			GLHandler::postProcess(shader, mainRenderTarget->sceneTarget,
			                       mainRenderTarget->postProcessingTargets[0]);
		}

		else
		{
			qDebug() << "Invalid MainRenderTarget::Projection";
		}

		// compute average luminance
		if(computeAverageLuminance)
		{
			lastFrameAverageLuminance
			    = mainRenderTarget->postProcessingTargets[0]
			          .getColorAttachmentTexture()
			          .getAverageLuminance();
		}

		// postprocess
		int i(0);
		for(auto it(postProcessingPipeline_.begin());
		    it != postProcessingPipeline_.end(); ++i, ++it)
		{
			window.applyPostProcShaderParams(
			    it->first, it->second,
			    mainRenderTarget->postProcessingTargets.at(i % 2));
			auto texs = window.getPostProcessingUniformTextures(
			    it->first, it->second,
			    mainRenderTarget->postProcessingTargets.at(i % 2));
			GLHandler::postProcess(
			    it->second, mainRenderTarget->postProcessingTargets.at(i % 2),
			    mainRenderTarget->postProcessingTargets.at((i + 1) % 2), texs);
		}
		mainRenderTarget->postProcessingTargets
		    .at(postProcessingPipeline_.size() % 2)
		    .bind();
		window.renderGui(
		    mainRenderTarget->postProcessingTargets.at(0).getSize());
		// blit result on screen
		mainRenderTarget->postProcessingTargets
		    .at(postProcessingPipeline_.size() % 2)
		    .showOnWindow(window);
	}
}

void Renderer::clean()
{
	if(!initialized)
	{
		return;
	}

	compass.reset();
	dbgCamera.reset();
	mainRenderTarget.reset();

	initialized = false;
}

Renderer::~Renderer()
{
	clean();
}
