#include "AbstractMainWin.hpp"

std::unique_ptr<VRHandler> newVRHandler()
{
	if(QSettings().value("vr/handler").toString() == "openvr")
	{
		return std::make_unique<OpenVRHandler>();
	}
	return std::make_unique<StereoBeamerHandler>();
}

AbstractMainWin::AbstractMainWin()
    : vrHandler(newVRHandler())
    , renderer(*this, *vrHandler)
{
	m_context.setFormat(this->format());
	m_context.create();

	const unsigned int secondariesNb(
	    QSettings().value("window/windefinitions").toStringList().size() - 1);
	for(unsigned int i(0); i < secondariesNb; ++i)
	{
		secondaryWindows.push_back(std::make_unique<RenderingWindow>(i + 1));
		secondaryWindows[i]->setFullscreen(secondaryWindows[i]->isFullscreen());
	}
}

double AbstractMainWin::getHorizontalFOV() const
{
	return renderer.getHorizontalFOV();
}

double AbstractMainWin::getVerticalFOV() const
{
	return renderer.getVerticalFOV();
}

void AbstractMainWin::setHorizontalFOV(double fov)
{
	QSettings().setValue("graphics/hfov", fov);
	renderer.updateFOV();
}

void AbstractMainWin::setVerticalFOV(double fov)
{
	QSettings().setValue("graphics/vfov", fov);
	renderer.updateFOV();
}

QVector3D AbstractMainWin::getVirtualCamShift() const
{
	return QSettings().value("vr/virtualcamshift").value<QVector3D>();
}

void AbstractMainWin::setVirtualCamShift(QVector3D const& virtualCamShift)
{
	QSettings().setValue("vr/virtualcamshift", virtualCamShift);
}

void AbstractMainWin::reloadPythonEngine()
{
	reloadPy = true;
	PythonQtHandler::closeConsole();
}

void AbstractMainWin::sendPythonScript(unsigned int toClientId,
                                       QString const& script) const
{
	networkManager->sendPythonScript(toClientId, script);
}

bool AbstractMainWin::vrIsEnabled() const
{
	return vrHandler->isEnabled();
}

void AbstractMainWin::setVR(bool vr)
{
	if(vrHandler->isEnabled() && !vr)
	{
		vrHandler->close();
	}
	else if(!vrHandler->isEnabled() && vr)
	{
		if(vrHandler->init(renderer, *toneMappingModel))
		{
			vrHandler->resetPos();
		}
	}
	if(vrIsEnabled())
	{
		PythonQtHandler::addObject("VRHandler", vrHandler.get());
	}
	else
	{
		PythonQtHandler::evalScript(
		    "if \"VRHandler\" in dir():\n\tdel VRHandler");
	}

	renderer.updateRenderTargets();
	reloadBloomTargets();
}

void AbstractMainWin::toggleVR()
{
	setVR(!vrIsEnabled());
}

void AbstractMainWin::takeScreenshot(QString path) const
{
	QImage screenshot(renderer.getLastFrame());
	if(path == "")
	{
		path = QFileDialog::getSaveFileName(
		    nullptr, tr("Save Screenshot"),
		    QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
		    tr("Images (*.png *.xpm *.jpg)"));
	}
	screenshot.convertToFormat(QImage::Format_RGB888).save(path);
}

bool AbstractMainWin::event(QEvent* e)
{
	if(e->type() == QEvent::UpdateRequest)
	{
		if(isExposed())
		{
			paintGL();
		}
		// animate continuously: schedule an update
		QCoreApplication::postEvent(this,
		                            qt_owned<QEvent>(QEvent::UpdateRequest));
		return true;
	}
	if(e->type() == QEvent::Type::Close)
	{
		shaderSelector->close();
		textureSelector->close();
		menuBar->close();
		dialog3dWheel->close();
		PythonQtHandler::closeConsole();
		for(auto const& w : secondaryWindows)
		{
			w->close();
		}
		if(quitOnClose)
		{
			QCoreApplication::quit();
		}
	}
	return QWindow::event(e);
}

void AbstractMainWin::resizeEvent(QResizeEvent* ev)
{
	RenderingWindow::resizeEvent(ev);
	renderer.updateRenderTargets();
	reloadBloomTargets();
}

void AbstractMainWin::actionEvent(BaseInputManager::Action const& a,
                                  bool pressed)
{
	RenderingWindow::actionEvent(a, pressed);

	if(!pressed)
	{
		return;
	}

	if(a.id == "toggledbgcam")
	{
		renderer.getDebugCamera().toggle();
	}
	else if(a.id == "togglewireframe")
	{
		toggleWireframe();
	}
	else if(a.id == "togglediagnostics")
	{
		toggleDiagnostics();
	}
	else if(a.id == "reloadpythonengine")
	{
		reloadPythonEngine();
	}
	else if(a.id == "togglepyconsole")
	{
		PythonQtHandler::toggleConsole();
	}
	else if(a.id == "togglevr")
	{
		toggleVR();
	}
	else if(a.id == "screenshot")
	{
		takeScreenshot();
	}
	else if(a.id == "autoexposure")
	{
		toneMappingModel->autoexposure = !toneMappingModel->autoexposure;
		if(toneMappingModel->autoexposure)
		{
			toneMappingModel->dynamicrange      = 1e4f;
			toneMappingModel->autoexposurecoeff = 1.f;
		}
	}
	else if(a.id == "exposureup")
	{
		if(toneMappingModel->autoexposure)
		{
			toneMappingModel->autoexposurecoeff *= 1.5f;
		}
		else
		{
			toneMappingModel->exposure *= 1.5f;
		}
	}
	else if(a.id == "exposuredown")
	{
		if(toneMappingModel->autoexposure)
		{
			toneMappingModel->autoexposurecoeff /= 1.5f;
		}
		else
		{
			toneMappingModel->exposure /= 1.5f;
		}
	}
	else if(a.id == "dynamicrangeup")
	{
		if(toneMappingModel->dynamicrange < 1e37)
		{
			toneMappingModel->dynamicrange *= 10.f;
			if(toneMappingModel->autoexposure)
			{
				toneMappingModel->autoexposurecoeff *= 10.f;
			}
			else
			{
				toneMappingModel->exposure *= 10.f;
			}
		}
	}
	else if(a.id == "dynamicrangedown")
	{
		if(toneMappingModel->dynamicrange > 1.f)
		{
			toneMappingModel->dynamicrange /= 10.f;
			if(toneMappingModel->autoexposure)
			{
				toneMappingModel->autoexposurecoeff /= 10.f;
			}
			else
			{
				toneMappingModel->exposure /= 10.f;
			}
		}
	}
}

void AbstractMainWin::vrEvent(VRHandler::Event const& e)
{
	switch(e.type)
	{
		case VRHandler::EventType::BUTTON_PRESSED:
			switch(e.button)
			{
				case VRHandler::Button::MENU:
					dialog3dWheel->showFromController(
					    *vrHandler->getController(e.side));
					break;
				default:
					break;
			}
			break;
		case VRHandler::EventType::BUTTON_UNPRESSED:
			switch(e.button)
			{
				case VRHandler::Button::MENU:
					dialog3dWheel->click(*vrHandler->getController(e.side));
					dialog3dWheel->hide();
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}

	dialog3dWheel->vrEvent(e);

	PythonQtHandler::evalScript(
	    "if \"vrEvent\" in dir():\n\tvrEvent("
	    + QString::number(static_cast<int>(e.type)) + ","
	    + QString::number(static_cast<int>(e.side)) + ", "
	    + QString::number(static_cast<int>(e.button)) + ")");
}

void AbstractMainWin::gamepadEvent(GamepadHandler::Event const& /*e*/) {}

void AbstractMainWin::setupPythonAPI()
{
	PythonQtHandler::addObject("HydrogenVR", this);
}

void AbstractMainWin::renderGui(QSize const& targetSize,
                                AdvancedPainter& painter)
{
	if(!diagnostics)
	{
		return;
	}

	painter.setPen(QPen{Qt::blue});
	painter.drawFunction({10, 10, 256, 256}, 0, fpsHistory.size(), 0, 120.f,
	                     [this](float x)
	                     {
		                     return fpsHistory.at(static_cast<int>(std::floor(
		                                              x + currentFrame + 1))
		                                          % fpsHistory.size());
	                     });
	painter.setPen(QPen{Qt::red});
	painter.drawRect(QRect{10, 10, 256, 256});

	QString timingsStr;
	for(auto const& pair : timingsNs)
	{
		timingsStr += pair.first + ": "
		              + QString::number(pair.second.first / 1.e6f) + "CPUms "
		              + QString::number(pair.second.second / 1.e6f) + "GPUms\n";
	}
	timingsStr += "Full frame (full loop): "
	              + QString::number(frameTiming * 1.e3f) + "ms\n";
	painter.drawText(
	    QRect{10, 276, targetSize.width() - 10, targetSize.height() - 276},
	    timingsStr);
	if(logProfAction->isChecked())
	{
		QFile profLog("profiling.txt");
		bool exists(profLog.exists());
		profLog.open(QIODevice::WriteOnly | QIODevice::Append);
		if(!exists)
		{
			profLog.write(QString("Full frame (full loop) (ms),").toLatin1());
			for(auto const& pair : timingsNs)
			{
				profLog.write(
				    (pair.first + " (CPUms)," + pair.first + " (GPUms),")
				        .toLatin1());
			}
			profLog.write(QString('\n').toLatin1());
		}
		profLog.write((QString::number(frameTiming * 1.e3f) + ',').toLatin1());
		for(auto const& pair : timingsNs)
		{
			QString timings;
			timings += QString::number(pair.second.first / 1.e6f) + ',';
			timings += QString::number(pair.second.second / 1.e6f) + ',';
			profLog.write(timings.toLatin1());
		}
		if(frameTiming > 0.015f)
		{
			profLog.write(QString("<--------------").toLatin1());
		}
		profLog.write(QString('\n').toLatin1());
	}
}

void AbstractMainWin::applyPostProcShaderParams(
    QString const& id, GLShaderProgram const& shader,
    GLFramebufferObject const& /*currentTarget*/) const
{
	if(id == "exposure")
	{
		shader.setUniform("exposure", toneMappingModel->exposure);
		shader.setUniform("dynamicrange", toneMappingModel->dynamicrange);
		shader.setUniform("purkinje", toneMappingModel->purkinje ? 1.f : 0.f);
		shader.setUniform("gamma", gamma);
		shader.setUniform("contrast", toneMappingModel->contrast);
	}
	else if(id == "colors")
	{
		shader.setUniform("gamma", gamma);
		shader.setUniform("contrast", toneMappingModel->contrast);
	}
	else
	{
		QString pyCmd("if \"applyPostProcShaderParams\" in "
		              "dir():\n\tapplyPostProcShaderParams(\""
		              + id + "\"," + shader.toStr() + ")");
		PythonQtHandler::evalScript(pyCmd);
	}
}

std::vector<GLComputeShader::TextureBinding>
    AbstractMainWin::getPostProcessingUniformTextures(
        QString const& id, GLShaderProgram const& /*shader*/,
        GLFramebufferObject const& currentTarget) const
{
	if(id == "bloom")
	{
		if(bloom)
		{
			// high luminosity pass
			GLComputeShader hlshader("highlumpass");
			GLHandler::postProcess(hlshader, currentTarget, *bloomTargets[0]);

			// blurring
			GLComputeShader blurshader("blur");
			for(unsigned int i = 0; i < 6;
			    i++) // always execute even number of times
			{
				blurshader.setUniform("dir", i % 2 == 0 ? QVector2D(1, 0)
				                                        : QVector2D(0, 1));
				GLHandler::postProcess(blurshader, *bloomTargets.at(i % 2),
				                       *bloomTargets.at((i + 1) % 2));
			}

			return {{bloomTargets[0]->getColorAttachmentTexture(),
			         GLComputeShader::DataAccessMode::R}};
		}
		GLHandler::beginRendering(*bloomTargets[0]);
		return {{bloomTargets[0]->getColorAttachmentTexture(),
		         GLComputeShader::DataAccessMode::R}};
	}
	return {};
}

void AbstractMainWin::toggleWireframe()
{
	setWireframe(!getWireframe());
}

void AbstractMainWin::toggleDiagnostics()
{
	setDiagnostics(!getDiagnostics());
}

void AbstractMainWin::initializeGL()
{
	m_context.makeCurrent(this);
	// Init GL
	GLHandler::init();
	// Init ToneMappingModel
	toneMappingModel = std::make_unique<ToneMappingModel>(*vrHandler);
	// Init Dialog3DWheel
	dialog3dWheel
	    = std::make_unique<Dialog3DWheel>(*vrHandler, *toneMappingModel);
	// Init Renderer
	renderer.init(*dialog3dWheel);
	// Init PythonQt
	initializePythonQt();
	// Init VR
	setVR(QSettings().value("vr/enabled").toBool());
	// Init libraries
	initLibraries();
	// Init NetworkManager
	networkManager = std::make_unique<NetworkManager>(constructNewState());

	qDebug() << "Using OpenGL " << format().majorVersion() << "."
	         << format().minorVersion() << format().profile() << '\n';

	if(vrHandler->isEnabled())
	{
		vrHandler->resetPos();
	}

	// init menuBar

	menuBar = std::make_unique<QMenuBar>();
	menuBar->setWindowFlags(Qt::X11BypassWindowManagerHint
	                        | Qt::MSWindowsFixedSizeDialogHint
	                        | Qt::FramelessWindowHint);

	auto file(menuBar->addMenu(tr("File")));
	file->addAction(tr("Close"), this, [this]() { this->close(); });

	auto engine(menuBar->addMenu(tr("HydrogenVR")));
	engine->addAction(tr("Explore Shaders..."), this,
	                  [this]() {
		                  this->shaderSelector->setVisible(
		                      !this->shaderSelector->isVisible());
	                  });
	engine->addAction(tr("Explore Textures..."), this,
	                  [this]() {
		                  this->textureSelector->setVisible(
		                      !this->textureSelector->isVisible());
	                  });

	logProfAction = engine->addAction(tr("Log Profiling Timings"));
	logProfAction->setCheckable(true);
	QFile profLog("profiling.txt");
	profLog.remove();

	menuBar->show();

	shaderSelector  = std::make_unique<ShaderSelector>();
	textureSelector = std::make_unique<TextureSelector>();

	// let user init
	initScene();

	//#ifdef Q_OS_WIN
	if(isFullscreen())
	{
		fullScreenTimer.start();
	}
	//#endif

	// Init Python engine
	setupPythonScripts();

	QMap<QString, QString> defines;
	if(QSettings().value("graphics/dithering").toBool())
	{
		defines["DITHERING"] = "0";
	}
	if(bloom)
	{
		defines["BLOOM"] = "0";
	}
	renderer.appendPostProcessingShader("exposure", "exposure", defines);
	if(bloom)
	{
		renderer.appendPostProcessingShader("bloom", "bloom");
		renderer.appendPostProcessingShader("colors", "colors", defines);
	}

	frameTimer.start();
	initialized = true;

	// BLOOM
	reloadBloomTargets();
}

void AbstractMainWin::initializePythonQt()
{
	PythonQtHandler::init();
	PythonQtHandler::addClass<int>("Side");
	PythonQtHandler::addObject("Side", make_qt_unique<PySide>(*this));
	PythonQtHandler::addClass<int>("PrimitiveType");
	PythonQtHandler::addObject("PrimitiveType",
	                           make_qt_unique<PyPrimitiveType>(*this));
	PythonQtHandler::addObject("GLHandler", make_qt_unique<GLHandler>(*this));
	PythonQtHandler::addObject("ToneMappingModel", toneMappingModel.get());
	PythonQtHandler::addWrapper<GLShaderProgramWrapper>();
	PythonQtHandler::addWrapper<GLMeshWrapper>();
}

void AbstractMainWin::reloadPythonQt()
{
	PythonQtHandler::clean();
	initializePythonQt();
	setupPythonScripts();
	reloadPy = false;
}

void AbstractMainWin::setupPythonScripts()
{
	setupPythonAPI();

	QString mainScriptRootDir(
	    "./data/" + QString(PROJECT_DIRECTORY) + "/scripts/"
	    + QSettings().value("scripting/rootdir").toString());

	if(!QSettings().value("scripting/customdir").toString().isEmpty())
	{
		mainScriptRootDir = QSettings().value("scripting/customdir").toString();
	}

	QString mainScriptPath(mainScriptRootDir + "/main.py");
	if(QFile(mainScriptPath).exists())
	{
		PythonQtHandler::evalFile(mainScriptPath);
	}

	PythonQtHandler::evalScript("if \"initScene\" in dir():\n\tinitScene()");
}

void AbstractMainWin::paintGL()
{
	m_context.makeCurrent(this);
	if(!initialized)
	{
		initializeGL();
	}

	Timings::start("Full frame (no swap)");

	if(!videomode)
	{
		frameTiming_ = frameTimer.nsecsElapsed() * 1.e-9f;
	}
	else
	{
		frameTiming_ = 1.f / QSettings().value("window/videofps").toUInt();
	}
	frameTimer.restart();

	// update fps history
	float curVal = 1.f / frameTiming;
	avgFPS
	    -= fpsHistory.at((currentFrame - avgFPSWindowSize) % fpsHistory.size())
	       / avgFPSWindowSize;
	avgFPS += curVal / avgFPSWindowSize;

	fpsHistory.at(currentFrame % fpsHistory.size()) = curVal;
	++currentFrame;
	currentFrame = currentFrame % fpsHistory.size();

	setTitle(QString(PROJECT_NAME) + " - " + QString::number(std::round(avgFPS))
	         + " FPS");

	// update menubar visibility
	auto cursorPos(QCursor::pos() - position());
	if(cursorPos.y() < menuBar->height() + 10 && cursorPos.y() >= 0
	   && cursorPos.x() >= 0 && cursorPos.x() < width())
	{
		if(!menuBar->isVisible())
		{
			menuBar->move(position());
			menuBar->show();
		}
	}
	else if(menuBar->activeAction() == nullptr && menuBar->isVisible())
	{
		menuBar->hide();
	}

	if(reloadPy)
	{
		reloadPythonQt();
	}
	if(vrHandler->isEnabled())
	{
		float vrFT(vrHandler->getFrameTiming());
		if(vrFT >= 0.f)
		{
			frameTiming_ = vrFT / 1000.f;
		}
	}

	auto* nState(networkManager->getNetworkedState());
	if(nState != nullptr)
	{
		if(networkManager->isServer())
		{
			writeState(*nState);
		}
		else
		{
			readState(*nState);
		}
		networkManager->update(frameTiming);
	}

	AsyncTexture::forceSync() = videomode;
	AsyncMesh::forceSync()    = videomode;

	toneMappingModel->autoUpdateExposure(
	    renderer.getLastFrameAverageLuminance(), frameTiming);

	//#ifdef Q_OS_WIN
	// fullscreen can be initially wrong on Windows, quick hack is to toggle it
	// back and forth
	// update : now also wrong on Linux
	if(fullScreenTimer.isValid())
	{
		if(fullScreenTimer.elapsed() / 1000 > 1 && isFullscreen())
		{
			if(!getScreenName().isEmpty())
			{
				setFullscreen(false);
			}
			for(auto const& secWin : secondaryWindows)
			{
				if(!secWin->getScreenName().isEmpty())
				{
					secWin->setFullscreen(false);
				}
			}
		}
		if(fullScreenTimer.elapsed() / 1000 > 2 && !isFullscreen())
		{
			if(!getScreenName().isEmpty())
			{
				setFullscreen(true);
			}
			for(auto const& secWin : secondaryWindows)
			{
				if(!secWin->getScreenName().isEmpty())
				{
					secWin->setFullscreen(true);
				}
			}
			fullScreenTimer.invalidate();
		}
	}
	//#endif

	// handle VR events if any
	if(vrHandler->isEnabled())
	{
		VRHandler::Event e{};
		while(vrHandler->pollEvent(e))
		{
			vrEvent(e);
		}
	}
	// handle gamepad events if any
	if(gamepadHandler.isEnabled())
	{
		GamepadHandler::Event e;
		while(gamepadHandler.pollEvent(e))
		{
			gamepadEvent(e);
		}
	}
	// let user update before rendering
	for(auto const& pair : renderer.sceneRenderPipeline)
	{
		updateScene(*pair.second.camera, pair.first);
	}
	PythonQtHandler::evalScript(
	    "if \"updateScene\" in dir():\n\tupdateScene()");

	if(renderer.getCalibrationCompass())
	{
		renderer.getCalibrationCompassPtr()->exposure
		    = toneMappingModel->exposure;
		renderer.getCalibrationCompassPtr()->dynamicrange
		    = toneMappingModel->dynamicrange;
	}

	// Render frame
	renderer.computeAverageLuminance = toneMappingModel->autoexposure;
	if(vrHandler->isEnabled())
	{
		vrHandler->forceLeft  = isForcedLeft();
		vrHandler->forceRight = isForcedRight();
	}
	renderer.renderFrame(getAngleShiftMatrix());
	for(auto const& w : secondaryWindows)
	{
		if(vrHandler->isEnabled())
		{
			vrHandler->forceLeft  = w->isForcedLeft();
			vrHandler->forceRight = w->isForcedRight();
		}
		m_context.makeCurrent(w.get());
		renderer.renderFrame(w->getAngleShiftMatrix());
		w->show();
	}
	// render texture display if available
	{
		auto viewer(textureSelector->getViewer());
		if(viewer != nullptr)
		{
			auto& win = viewer->getTextureDisplayWindow();
			if(win.isExposed())
			{
				m_context.makeCurrent(&win);
				viewer->render();
				m_context.swapBuffers(&win);
			}
		}
	}
	m_context.makeCurrent(this);

	// garbage collect some resources
	AsyncTexture::garbageCollect();

	if(videomode)
	{
		if(!videoRenderingTimer.isValid() && currentVideoFrame > 1)
		{
			videoRenderingTimer.start();
		}
		QImage frame(renderer.getLastFrame());
		QString number
		    = QString("%1").arg(currentVideoFrame, 5, 10, QChar('0'));

		QString subdir;
		switch(renderer.projection)
		{
			case MainRenderTarget::Projection::DEFAULT:
				subdir = "2D";
				break;
			case MainRenderTarget::Projection::PANORAMA360:
				subdir = "PANORAMA360";
				break;
			case MainRenderTarget::Projection::VR180L:
				subdir = "VR180L";
				break;
			case MainRenderTarget::Projection::VR180R:
				subdir = "VR180R";
				break;
			case MainRenderTarget::Projection::VR180:
				subdir = "VR180";
				break;
			case MainRenderTarget::Projection::DOMEMASTER180:
				subdir = "DOMEMASTER180";
				break;
		}

		QString res
		    = QString::number(renderer.getSize().width()) + "x"
		      + QString::number(renderer.getSize().height()) + "_"
		      + QString::number(QSettings().value("window/videofps").toInt())
		      + "fps";
		if(currentVideoFrame == 0)
		{
			QDir viddir(QSettings().value("window/viddir").toString());
			viddir.mkpath(".");
			QFile::copy("./" + getAbsoluteDataPath("scripts/generate_vids.sh"),
			            QSettings().value("window/viddir").toString()
			                + "/generate_vids.sh");
			viddir.mkdir(subdir);
			QDir projdir(QSettings().value("window/viddir").toString() + "/"
			             + subdir);
			projdir.mkdir(res);
			QDir framesdir = QSettings().value("window/viddir").toString() + "/"
			                 + subdir + "/" + res;
			framesdir.setNameFilters(QStringList() << "frame*.png");
			framesdir.setFilter(QDir::Files);
			for(auto const& f : framesdir.entryList())
			{
				framesdir.remove(f);
			}
		}
		unsigned int maxframe(QSettings().value("window/maxframe").toUInt());
		if(maxframe > 0)
		{
			if(currentVideoFrame > maxframe)
			{
				QString resultStr("Rendered ");
				resultStr += QString::number(maxframe) + " frames in ";
				QTime t(0, 0, 0);
				t = t.addSecs(videoRenderingTimer.elapsed() / 1000);
				resultStr += t.toString();
				qDebug() << resultStr;
				close();
			}
			else if(currentVideoFrame != 0)
			{
				QString progressStr("Progress : ");
				progressStr += QString::number(currentVideoFrame) + "/"
				               + QString::number(maxframe)
				               + " | Time remaining: ";
				int elapsed(videoRenderingTimer.elapsed() / 1000);
				int remaining(elapsed * (maxframe - currentVideoFrame)
				              / currentVideoFrame);
				if(remaining > 3600 * 24)
				{
					progressStr += QString::number(
					                   std::floor(remaining / (3600.f * 24.f)))
					               + "d";
				}

				QTime t(0, 0, 0);
				t = t.addSecs(remaining);
				progressStr += t.toString();
				qDebug() << progressStr;
			}
		}

		QString framePath(QSettings().value("window/viddir").toString() + "/"
		                  + subdir + "/" + res + "/frame" + number + ".png");
		qDebug() << "Writing " + framePath + "...";
		QThreadPool::globalInstance()->start(qt_owned<ImageWriter>(
		    framePath, frame.convertToFormat(QImage::Format_RGB888)));

		currentVideoFrame++;
	}

	Timings::end("Full frame (no swap)");
	// Trigger a repaint immediatly
	m_context.swapBuffers(this);
	for(auto const& w : secondaryWindows)
	{
		m_context.swapBuffers(w.get());
	}

	// Retrieve GPU timings after swapBuffers to use swapBuffers as the CPU/GPU
	// sync, otherwise we would need to wait for the result then swapBuffers.
	timingsNs = Timings::getTimingsNanosecond();
}

AbstractMainWin::~AbstractMainWin()
{
	// force garbage collect some resources
	AsyncTexture::garbageCollect(true);

	PythonQtHandler::evalScript(
	    "if \"cleanUpScene\" in dir():\n\tcleanUpScene()");
	vrHandler->close();
	PythonQtHandler::clean();

	Timings::cleanUp();
}

void AbstractMainWin::reloadBloomTargets()
{
	if(!initialized || !bloom)
	{
		return;
	}
	bloomTargets[0] = std::make_unique<GLFramebufferObject>(
	    GLTexture::Tex2DProperties(renderer.getSize().width(),
	                               renderer.getSize().height(), GL_RGBA32F));
	bloomTargets[1] = std::make_unique<GLFramebufferObject>(
	    GLTexture::Tex2DProperties(renderer.getSize().width(),
	                               renderer.getSize().height(), GL_RGBA32F));
}
