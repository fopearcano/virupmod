#include "MainWin.hpp"

MainWin::MainWin()
{
	srand(time(nullptr));
}

void MainWin::actionEvent(BaseInputManager::Action a, bool pressed)
{
	if(loaded)
	{
		if(pressed)
		{
			if(a.id == "resetvrpos")
			{
				// integralDt    = 0;
				if(vrHandler->isEnabled())
				{
					vrHandler->resetPos();
				}
			}
			else if(a.id == "toggleorbits")
			{
				CelestialBodyRenderer::renderOrbits
				    = CelestialBodyRenderer::renderOrbits > 0.f ? 0.f : 1.f;
			}
			else if(a.id == "togglelabels")
			{
				CelestialBodyRenderer::renderLabels
				    = CelestialBodyRenderer::renderLabels > 0.f ? 0.f : 1.f;
			}
			else if(a.id == "toggledm")
			{
				Method::toggleDarkMatter();
			}
			else if(a.id == "togglegrid")
			{
				setGridEnabled(!gridEnabled());
			}
			/*else if(e->key() == Qt::Key_H)
			{
			    setHDR(!getHDR());
			}*/
			else if(a.id == "showposition")
			{
				printPositionInDataSpace();
			}
			else if(a.id == "timecoeffdown")
			{
				float tc(universe->getTimeCoeff());
				if(tc > 1.f && !universe->getLockedRealTime())
				{
					universe->setTimeCoeff(tc / 10.f);
					debugText->setText(
					    ("Time coeff. : "
					     + std::to_string(static_cast<int>(tc / 10.f)) + "x")
					        .c_str());
					timeSinceTextUpdate = 0.f;
				}
			}
			else if(a.id == "timecoeffup")
			{
				float tc(universe->getTimeCoeff());
				if(tc < 1000000.f && !universe->getLockedRealTime())
				{
					universe->setTimeCoeff(tc * 10.f);
					debugText->setText(
					    ("Time coeff. : "
					     + std::to_string(static_cast<int>(tc * 10.f)) + "x")
					        .c_str());
					timeSinceTextUpdate = 0.f;
				}
			}
		}
		movementControls->actionEvent(a, pressed);
	}
	AbstractMainWin::actionEvent(a, pressed);
}

bool MainWin::event(QEvent* e)
{
	if(e->type() == QEvent::Type::Close)
	{
		if(visibilities != nullptr)
		{
			visibilities->close();
		}
		if(planetSysSelect != nullptr)
		{
			planetSysSelect->close();
		}
		if(univElemSelect != nullptr)
		{
			univElemSelect->close();
		}
		if(scenes != nullptr)
		{
			scenes->close();
		}
	}
	return AbstractMainWin::event(e);
}

void MainWin::mousePressEvent(QMouseEvent* e)
{
	if(e->button() == Qt::MouseButton::LeftButton)
	{
		moveView = true;
		QCursor c(cursor());
		c.setShape(Qt::CursorShape::BlankCursor);
		cursorPosBackup = QCursor::pos();
		QCursor::setPos(x() + width() / 2, y() + height() / 2);
		setCursor(c);
	}
}

void MainWin::mouseReleaseEvent(QMouseEvent* e)
{
	if(e->button() == Qt::MouseButton::LeftButton)
	{
		moveView = false;
		QCursor c(cursor());
		c.setShape(Qt::CursorShape::ArrowCursor);
		QCursor::setPos(cursorPosBackup);
		setCursor(c);
	}
}

void MainWin::mouseMoveEvent(QMouseEvent* e)
{
	if(!isActive() || vrHandler->isEnabled() || !loaded || !moveView)
	{
		cursorTimer.restart();
		auto c(cursor());
		if(c.shape() == Qt::CursorShape::BlankCursor)
		{
			c.setShape(Qt::CursorShape::ArrowCursor);
			setCursor(c);
		}
		return;
	}
	float dx = (x() + static_cast<float>(width()) / 2 - e->globalX()) / width();
	float dy
	    = (y() + static_cast<float>(height()) / 2 - e->globalY()) / height();
	auto& cam(renderer.getCamera<Camera>("cosmo"));
	cam.yaw += dx * 3.14f / 3.f;
	cam.pitch += dy * 3.14f / 3.f;
	auto& cam2(renderer.getCamera<OrbitalSystemCamera>("planet"));
	cam2.yaw += dx * 3.14f / 3.f;
	cam2.pitch += dy * 3.14f / 3.f;
	QCursor::setPos(x() + width() / 2, y() + height() / 2);
}

void MainWin::wheelEvent(QWheelEvent* e)
{
	if(loaded)
	{
		movementControls->wheelEvent(e);
	}
	AbstractMainWin::wheelEvent(e);
}

void MainWin::vrEvent(VRHandler::Event const& e)
{
	if(loaded)
	{
		switch(e.type)
		{
			case VRHandler::EventType::BUTTON_PRESSED:
				switch(e.button)
				{
					case VRHandler::Button::TOUCHPAD:
					{
						Controller const* ctrl(
						    vrHandler->getController(e.side));
						if(ctrl != nullptr)
						{
							QVector2D padCoords(ctrl->getPadCoords());
							if(fabsf(padCoords[0])
							   > fabsf(padCoords[1])) // LEFT OR RIGHT
							{
								if(padCoords[0] < 0.0f) // LEFT
								{
									toneMappingModel->exposure *= 8.0 / 10.0;
								}
								else // RIGHT
								{
									toneMappingModel->exposure *= 10.0 / 8.0;
								}
							}
							else // UP OR DOWN
							{
								float tc(universe->getTimeCoeff());
								if(padCoords[1] < 0.0f) // DOWN
								{
									if(tc > 1.f
									   && !universe->getLockedRealTime())
									{
										universe->setTimeCoeff(tc / 10.f);
										debugText->setText(
										    ("Time coeff. : "
										     + std::to_string(
										           static_cast<int>(tc / 10.f))
										     + "x")
										        .c_str());
										timeSinceTextUpdate = 0.f;
									}
								}
								else // UP
								{
									if(tc < 1000000.f
									   && !universe->getLockedRealTime())
									{
										universe->setTimeCoeff(tc * 10.f);
										debugText->setText(
										    ("Time coeff. : "
										     + std::to_string(
										           static_cast<int>(tc * 10.f))
										     + "x")
										        .c_str());
										timeSinceTextUpdate = 0.f;
									}
								}
							}
						}
						break;
					}
					default:
						break;
				}
				break;
			default:
				break;
		}

		movementControls->vrEvent(
		    e, renderer.getCamera("cosmo").seatedTrackedSpaceToWorldTransform(),
		    universe->isPlanetarySystemRendered());
	}
	AbstractMainWin::vrEvent(e);
}

void MainWin::gamepadEvent(GamepadHandler::Event const& e)
{
	if(loaded)
	{
		movementControls->gamepadEvent(e);
	}
	if(e.type == GamepadHandler::EventType::BUTTON_PRESSED)
	{
		bool sceneChanged = true;
		switch(e.button)
		{
			case GamepadHandler::Button::A:
				animator->recenter();
				QSound::play(
				    getAbsoluteDataPath("sounds/buttons/recenter.wav"));
				break;
			case GamepadHandler::Button::B:
				animator->next();
				QSound::play(getAbsoluteDataPath("sounds/buttons/next.wav"));
				break;
			case GamepadHandler::Button::X:
				animator->previous();
				QSound::play(
				    getAbsoluteDataPath("sounds/buttons/previous.wav"));
				break;
			case GamepadHandler::Button::Y:
				animator->home();
				QSound::play(getAbsoluteDataPath("sounds/buttons/home.wav"));
				break;
			default:
				sceneChanged = false;
				break;
		}
		if(sceneChanged)
		{
			auto id(animator->getCurrentTransitionId());
			if(id >= 0
			   && static_cast<unsigned int>(id)
			          < animator->getTransitions().size())
			{
				timeSinceTextUpdate = 0.0;
				debugText->setText(
				    QString(tr("Going to : "))
				    + animator
				          ->getTransitions()[animator->getCurrentTransitionId()]
				          .getName());
			}
		}
	}
	AbstractMainWin::gamepadEvent(e);
}

void MainWin::setupPythonAPI()
{
	PythonQtHandler::addObject("VIRUP", this);
	PythonQtHandler::addWrapper<TransitionWrapper>();
	PythonQtHandler::addWrapper<SceneWrapper>();
	PythonQtHandler::addWrapper<SceneSpatialDataWrapper>();
	PythonQtHandler::addWrapper<SceneTemporalDataWrapper>();
	PythonQtHandler::addWrapper<SceneUIWrapper>();
}

void MainWin::initLibraries()
{
	initLibrary<LibPlanet>();
}

void MainWin::initScene()
{
	toneMappingModel->exposure     = 0.3f;
	toneMappingModel->dynamicrange = 10000.f;
	grid                           = new Grid;

	auto cam            = new Camera(*vrHandler);
	cam->seatedVROrigin = false;
	cam->setPerspectiveProj(renderer.getVerticalFOV(),
	                        renderer.getAspectRatioFromFOV());

	auto camPlanet = new OrbitalSystemCamera(
	    *vrHandler, toneMappingModel->exposure, toneMappingModel->dynamicrange);
	camPlanet->seatedVROrigin = false;
	camPlanet->setPerspectiveProj(renderer.getVerticalFOV(),
	                              renderer.getAspectRatioFromFOV());

	// COSMO LOADING
	universe = new Universe(*cam, *camPlanet);

	// DEBUG TEXT
	debugText = new Text3D(textWidth, textHeight);
	debugText->setFlags(Qt::AlignCenter);
	debugText->setColor(
	    QSettings().value("misc/uilabelscolor").value<QColor>());
	debugText->setText("");

	QString fontPath(QSettings().value("misc/uilabelsfont").toString());
	if(!fontPath.isEmpty())
	{
		auto id         = QFontDatabase::addApplicationFont(fontPath);
		auto fontFamily = QFontDatabase::applicationFontFamilies(id)[0];
		if(!fontFamily.isEmpty())
		{
			QFont f = debugText->getFont();
			f.setFamily(fontFamily);
			debugText->setFont(f);
		}
	}
	debugText->setSuperSampling(2.f);

	movementControls = new MovementControls(
	    *vrHandler, universe->getBoundingBox(), cam, camPlanet);

	renderer.removeSceneRenderPath("default");

	renderer.appendSceneRenderPath("cosmo", Renderer::RenderPath(cam));
	renderer.appendSceneRenderPath("planet", Renderer::RenderPath(camPlanet));

	animator = new Animator(*universe, *vrHandler, *toneMappingModel);

	// we will draw them ourselves
	renderer.pathIdRenderingControllers = "";

	loaded = true;

	// LENSING
	lenseDistortionMap
	    = new GLTexture("data/virup/images/pointmass-distortion.png", false);

	// renderer.appendPostProcessingShader("lensing", "lensing");

	// UI
	if(networkManager->isServer())
	{
		visibilities    = new Visibilities(*universe);
		planetSysSelect = new PlanetarySystemSelector(*universe, *animator);
		univElemSelect  = new UniverseElementSelector(*universe, *animator);
		scenes          = new SceneSelector(*animator);

		/*dialog3dWheel->addDialog3D(tr("Scenes"), *scenes);
		dialog3dWheel->addDialog3D(tr("Universe Elements"), *univElemSelect);
		dialog3dWheel->addDialog3D(tr("Planetary Systems"), *planetSysSelect);
		dialog3dWheel->addDialog3D(tr("Visibilities List"), *visibilities);*/

		auto tools(menuBar->addMenu(tr("Tools")));
		tools->addAction(tr("Scenes"), this,
		                 [this]() { this->scenes->show(); });
		tools->addAction(tr("Universe Elements"), this,
		                 [this]() { this->univElemSelect->show(); });
		tools->addAction(tr("Planetary Systems"), this,
		                 [this]() { this->planetSysSelect->show(); });
		tools->addAction(tr("Visibilities List"), this,
		                 [this]() { this->visibilities->show(); });
	}
	cursorTimer.start();
}

void MainWin::updateScene(BasicCamera& camera, QString const& pathId)
{
	if(!loaded)
	{
		return;
	}

	if(pathId == "cosmo")
	{
		if(cursorTimer.elapsed() > 2000
		   && cursor().shape() == Qt::CursorShape::ArrowCursor)
		{
			auto c(cursor());
			c.setShape(Qt::CursorShape::BlankCursor);
			setCursor(c);
		}
		if(networkManager->isServer())
		{
			animator->update();
		}

		auto& cam(dynamic_cast<Camera&>(camera));
		cam.currentFrameTiming = frameTiming;
		cam.currentProjection  = renderer.projection;
		cam.updateTargetFPS();

		if(videomode)
		{
			OctreeLOD::forceMaxQuality() = true;
		}
		OrbitRenderer::forceRedraw = videomode;

		universe->updateCosmo();

		if(gamepadHandler.isEnabled())
		{
			auto rightJoystick(gamepadHandler.getJoystick(Side::RIGHT));
			float yaw(universe->getCamYaw()), pitch(universe->getCamPitch());
			yaw -= 2.0 * rightJoystick.x() * frameTiming;
			pitch += 2.0 * rightJoystick.y() * frameTiming;
			universe->setCamYaw(yaw);
			universe->setCamPitch(pitch);
		}
		movementControls->update(
		    frameTiming, universe->isPlanetarySystemRendered(), gamepadHandler);

		if(networkManager->isServer())
		{
			scenes->update();
		}
	}
	if(pathId == "planet")
	{
		auto& cam = dynamic_cast<OrbitalSystemCamera&>(camera);
		if(vrHandler->isEnabled() && vrHandler->getDriverName() == "OpenVR")
		{
			QVector3D pos(0.f, -0.15f, -0.4f);
			pos *= QSettings().value("misc/uilabelsdistmul").toDouble();
			debugText->getModel() = cam.hmdSpaceToWorldTransform();
			debugText->getModel().translate(pos);
			debugText->getModel().scale(
			    1.5 * static_cast<float>(textWidth) / width(),
			    1.5 * static_cast<float>(textHeight) / height());
		}
		else
		{
			QVector3D pos(0.f, -0.15f, -0.5f);
			pos *= QSettings().value("misc/uilabelsdistmul").toDouble();
			debugText->getModel() = cam.cameraSpaceToWorldTransform();
			debugText->getModel().translate(pos);
			debugText->getModel().scale(
			    2 * static_cast<float>(textWidth) / width(),
			    2 * static_cast<float>(textWidth) / height());
		}
		debugText->getShader().setUniform("exposure",
		                                  toneMappingModel->exposure);
		debugText->getShader().setUniform("dynamicrange",
		                                  toneMappingModel->dynamicrange);

		universe->updateClock(videomode, frameTiming);

		universe->updatePlanetarySystem();
		timeSinceTextUpdate += frameTiming;

		if(!universe->planetSystems->renderSystem())
		{
			return;
		}
		std::string targetName(cam.target->getName());
		/*
		if(targetName != lastTargetName)
		{
		    debugText->setText(QString("Locked to ") + targetName.c_str());
		    timeSinceTextUpdate = 0.0;
		    lastTargetName      = targetName;
		}
		*/
	}
}

void MainWin::renderScene(BasicCamera const& camera, QString const& pathId)
{
	if(!loaded)
	{
		return;
	}

	if(pathId == "planet")
	{
		if(!universe->isPlanetarySystemRendered())
		{
			if(timeSinceTextUpdate < 5.0)
			{
				debugText->render();
			}
		}
		else
		{
			universe->renderPlanetarySystem();
			renderer.renderVRControls();
			universe->renderPlanetarySystemTransparent();
			if(timeSinceTextUpdate < 5.0)
			{
				debugText->render();
			}
		}
		if(showGrid)
		{
			grid->render(universe->getScale(), 1.125);
		}
		movementControls->renderGuides();

		if(vrHandler->isEnabled())
		{
			QMatrix4x4 model;
			QSizeF playAreaSize(vrHandler->getPlayAreaSize());
			if(playAreaSize.width() > playAreaSize.height())
			{
				model.translate(-0.5f * playAreaSize.width() + 0.45, 0.f, 0.f);
			}
			else
			{
				model.translate(0.f, 0.f, -0.5f * playAreaSize.height() + 0.45);
				model.rotate(-90.f, 0.f, 1.f, 0.f);
			}
		}
		return;
	}

	if(!universe->isPlanetarySystemRendered())
	{
		renderer.renderVRControls();
	}
	auto& cam(dynamic_cast<Camera const&>(camera));

	universe->renderCosmo(*toneMappingModel);

	// update here because depends on eye
	QVector3D pos(
	    Utils::toQt(cam.dataToWorldPosition(Vector3(0.43, 8.24, 0.81))));
	lenseScreenCoord = camera.project(pos);
	lenseScreenCoord /= lenseScreenCoord.w();
	lenseDist
	    = ((camera.hmdScaledSpaceToWorldTransform() * QVector3D(0, 0, 0)) - pos)
	          .length();
}

void MainWin::applyPostProcShaderParams(
    QString const& id, GLShaderProgram const& shader,
    GLFramebufferObject const& currentTarget) const
{
	AbstractMainWin::applyPostProcShaderParams(id, shader, currentTarget);
	if(id == "lensing")
	{
		float aspectRatio(renderer.getAspectRatioFromFOV());
		if(vrHandler->isEnabled() && vrHandler->getDriverName() == "OpenVR")
		{
			QSize rtSize(vrHandler->getEyeRenderTargetSize());
			aspectRatio = rtSize.width();
			aspectRatio /= rtSize.height();
		}

		shader.setUniform("aspectRatio", aspectRatio);
		shader.setUniform("lenseSize",
		                  static_cast<float>(1.0e14 * universe->getScale()));
		shader.setUniform("lenseScreenCoord", lenseScreenCoord);
		shader.setUniform("lenseDist", lenseDist);
		shader.setUniform("radiusLimit", 0.2f);

		shader.setUniform("distortionMap", 1);
	}
}

std::vector<std::pair<GLTexture const*, GLComputeShader::DataAccessMode>>
    MainWin::getPostProcessingUniformTextures(
        QString const& id, GLShaderProgram const& shader,
        GLFramebufferObject const& currentTarget) const
{
	auto abstractResult(AbstractMainWin::getPostProcessingUniformTextures(
	    id, shader, currentTarget));
	if(!abstractResult.empty())
	{
		return abstractResult;
	}
	if(id == "lensing")
	{
		return {{lenseDistortionMap, GLComputeShader::DataAccessMode::R}};
	}
	return {};
}

void MainWin::printPositionInDataSpace(Side controller) const
{
	QVector3D position(0.f, 0.f, 0.f);
	Controller const* cont(vrHandler->getController(controller));
	// world space first
	if(cont != nullptr)
	{
		position
		    = renderer.getCamera("cosmo").seatedTrackedSpaceToWorldTransform()
		      * cont->getPosition();
	}
	else
	{
		position = renderer.getCamera("cosmo").hmdScaledSpaceToWorldTransform()
		           * position;
	}

	// then data space
	position
	    = Utils::toQt(renderer.getCamera<Camera>("cosmo").worldToDataPosition(
	        Utils::fromQt(position)));
	QString posstr;
	&posstr << position;

	auto msgBox = new QMessageBox;
	msgBox->setAttribute(Qt::WA_DeleteOnClose);
	msgBox->setStandardButtons(QMessageBox::Ok);
	msgBox->setWindowTitle(tr("Position selected"));
	msgBox->setText(posstr);
	msgBox->setModal(false);
	msgBox->show();
}

std::vector<float> MainWin::generateVertices(unsigned int number,
                                             unsigned int seed)
{
	std::vector<float> vertices;
	vertices.reserve(3 * number);

	srand(seed);

	for(unsigned int i(0); i < 3 * number; ++i)
	{
		vertices.push_back(
		    // NOLINT(cert-msc30-c)
		    2 * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX))
		    - 1);
	}

	return vertices;
}

MainWin::~MainWin()
{
	delete animator;
	delete scenes;
	delete visibilities;
	delete planetSysSelect;
	delete univElemSelect;
	delete lenseDistortionMap;
	delete debugText;
	delete movementControls;
	delete universe;
	delete grid;
}
