#include "MainWin.hpp"

#include <QOpenGLPaintDevice>

#include "LibTerrain.hpp"

void MainWin::actionEvent(BaseInputManager::Action const& a, bool pressed,
                          bool autorepeated)
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
			else if(a.id == "toggleinfotext")
			{
				showInfoText = !showInfoText;
			}
			else if(a.id == "toggleorbits")
			{
				CelestialBodyRenderer::renderOrbits()
				    = CelestialBodyRenderer::renderOrbits() > 0.f ? 0.f : 1.f;
			}
			else if(a.id == "togglelabels")
			{
				CelestialBodyRenderer::renderLabels()
				    = CelestialBodyRenderer::renderLabels() > 0.f ? 0.f : 1.f;
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
				const float tc(universe->getTimeCoeff());
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
				const float tc(universe->getTimeCoeff());
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
	AbstractMainWin::actionEvent(a, pressed, autorepeated);
}

bool MainWin::event(QEvent* e)
{
	if(e->type() == QEvent::Type::Close)
	{
		if(visibilities != nullptr)
		{
			visibilities->close();
		}
		if(timeController != nullptr)
		{
			timeController->close();
		}
		if(animTimeSelect != nullptr)
		{
			animTimeSelect->close();
		}
		if(tmController != nullptr)
		{
			tmController->close();
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
		if(presenterHelp != nullptr)
		{
			presenterHelp->close();
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	const float dx
	    = (x() + static_cast<float>(width()) / 2 - e->globalPosition().x())
	      / width();
	const float dy
	    = (y() + static_cast<float>(height()) / 2 - e->globalPosition().y())
	      / height();
#else
	const float dx
	    = (x() + static_cast<float>(width()) / 2 - e->globalX()) / width();
	const float dy
	    = (y() + static_cast<float>(height()) / 2 - e->globalY()) / height();
#endif
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
								const float tc(universe->getTimeCoeff());
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
		stopIdle();
		bool sceneChanged = true;
		switch(e.button)
		{
			case GamepadHandler::Button::A:
				animator->recenter();
				recenterSound.play();
				break;
			case GamepadHandler::Button::B:
				animator->next();
				nextSound.play();
				break;
			case GamepadHandler::Button::X:
				animator->previous();
				previousSound.play();
				break;
			case GamepadHandler::Button::Y:
				animator->home();
				homeSound.play();
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
				    animator
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
	PythonQtHandler::addWrapper<SceneCameraDataWrapper>();
	PythonQtHandler::addWrapper<SceneToneMappingDataWrapper>();
}

void MainWin::initLibraries()
{
	initLibrary<LibPlanet>();
	initLibrary<trn::LibTerrain>();
}

void MainWin::initScene()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	// allow reading large images
	QImageReader::setAllocationLimit(0);
#endif

	toneMappingModel->exposure     = 0.3f;
	toneMappingModel->dynamicrange = 10000.f;
	grid                           = std::make_unique<Grid>();

	std::unique_ptr<Camera> cam = std::make_unique<Camera>(*vrHandler);
	cam->seatedVROrigin         = false;
	cam->setPerspectiveProj(renderer.getVerticalFOV(),
	                        renderer.getAspectRatioFromFOV());

	std::unique_ptr<OrbitalSystemCamera> camPlanet
	    = std::make_unique<OrbitalSystemCamera>(*vrHandler,
	                                            toneMappingModel->exposure,
	                                            toneMappingModel->dynamicrange);
	camPlanet->seatedVROrigin = false;
	camPlanet->setPerspectiveProj(renderer.getVerticalFOV(),
	                              renderer.getAspectRatioFromFOV());

	// COSMO LOADING
	universe = std::make_unique<Universe>(*cam, *camPlanet);

	// UI
	helperBillboard = std::make_unique<Billboard>(
	    utils::getAbsoluteDataPath("images/halfcave_helper.png")
	        .toLatin1()
	        .data());

	debugText = std::make_unique<Text3D>(textWidth, textHeight);
	debugText->setFlags(Qt::AlignCenter);
	debugText->setColor(
	    QSettings().value("misc/uilabelscolor").value<QColor>());
	debugText->setText("");

	const QString fontPath(QSettings().value("misc/uilabelsfont").toString());
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

	movementControls = std::make_unique<MovementControls>(
	    *vrHandler, universe->getBoundingBox(), *cam, *camPlanet);

	inSound.setSource(QUrl::fromLocalFile(
	    utils::getAbsoluteDataPath("sounds/thruster/in.wav")));
	thrustSound.setSource(QUrl::fromLocalFile(
	    utils::getAbsoluteDataPath("sounds/thruster/thrust.wav")));
	outSound.setSource(QUrl::fromLocalFile(
	    utils::getAbsoluteDataPath("sounds/thruster/out.wav")));
	thrustSound.setLoopCount(QSoundEffect::Infinite);
	connect(&inSound, &QSoundEffect::playingChanged,
	        [this]()
	        {
		        if(!inSound.isPlaying())
		        {
			        thrustSound.play();
		        }
	        });
	connect(movementControls.get(), &MovementControls::gamepadIsMovingChanged,
	        [this](bool isMoving)
	        {
		        if(isMoving)
		        {
			        inSound.play();
		        }
		        else
		        {
			        thrustSound.stop();
			        outSound.play();
		        }
	        });
	recenterSound.setSource(QUrl::fromLocalFile(
	    utils::getAbsoluteDataPath("sounds/buttons/recenter.wav")));
	nextSound.setSource(QUrl::fromLocalFile(
	    utils::getAbsoluteDataPath("sounds/buttons/next.wav")));
	previousSound.setSource(QUrl::fromLocalFile(
	    utils::getAbsoluteDataPath("sounds/buttons/previous.wav")));
	homeSound.setSource(QUrl::fromLocalFile(
	    utils::getAbsoluteDataPath("sounds/buttons/home.wav")));

	renderer.removeSceneRenderPath("default");

	renderer.appendSceneRenderPath("cosmo",
	                               Renderer::RenderPath(std::move(cam)));
	renderer.appendSceneRenderPath("planet",
	                               Renderer::RenderPath(std::move(camPlanet)));

	animator = std::make_unique<Animator>(*universe, *vrHandler,
	                                      *toneMappingModel, *this);

	// we will draw them ourselves
	renderer.pathIdRenderingControllers = "";

	loaded = true;

	// LENSING
	lenseDistortionMap = std::make_unique<GLTexture>(
	    "data/virup/images/pointmass-distortion.png", false);
	lenseDistortionMap->generateMipmap();

	// UI
	if(networkManager->isServer())
	{
		visibilities = std::make_unique<Visibilities>(*universe);
		planetSysSelect
		    = std::make_unique<PlanetarySystemSelector>(*universe, *animator);
		univElemSelect
		    = std::make_unique<UniverseElementSelector>(*universe, *animator);
		timeController = std::make_unique<TimeController>(*universe);
		animTimeSelect = std::make_unique<AnimationTimeSelector>(*universe);
		tmController
		    = std::make_unique<ToneMappingController>(*toneMappingModel);
		scenes        = std::make_unique<SceneSelector>(*animator);
		presenterHelp = std::make_unique<PresenterHelp>(*this);

		/*dialog3dWheel->addDialog3D(tr("Scenes"), *scenes);
		dialog3dWheel->addDialog3D(tr("Universe Elements"), *univElemSelect);
		dialog3dWheel->addDialog3D(tr("Planetary Systems"), *planetSysSelect);
		dialog3dWheel->addDialog3D(tr("Time Controller"), *timeController);
		dialog3dWheel->addDialog3D(tr("Animation Time Controller"),
		*animTimeSelect); dialog3dWheel->addDialog3D(tr("Time Controller"),
		*tmController); dialog3dWheel->addDialog3D(tr("Visibilities List"),
		*visibilities);*/

		auto* tools(menuBar->addMenu(tr("Tools")));
		tools->addAction(tr("Scenes"), this,
		                 [this]() { this->scenes->show(); });
		tools->addAction(tr("Universe Elements"), this,
		                 [this]() { this->univElemSelect->show(); });
		tools->addAction(tr("Planetary Systems"), this,
		                 [this]() { this->planetSysSelect->show(); });
		tools->addAction(tr("Time Controller"), this,
		                 [this]() { this->timeController->show(); });
		tools->addAction(tr("Animation Time Controller"), this,
		                 [this]() { this->animTimeSelect->show(); });
		tools->addAction(tr("Tone Mapping Controller"), this,
		                 [this]() { this->tmController->show(); });
		tools->addAction(tr("Visibilities List"), this,
		                 [this]() { this->visibilities->show(); });
	}
	if(!QSettings().value("misc/presenterscreen").toString().isEmpty())
	{
		visibilities->show();
		planetSysSelect->show();
		univElemSelect->show();
		timeController->show();
		animTimeSelect->show();
		tmController->show();
		scenes->show();
		presenterHelp->show();
	}
	cursorTimer.start();

	// AMBIANCE

	ambiance.setSource(
	    QUrl::fromLocalFile(utils::getAbsoluteDataPath("sounds/music/00.wav")));
	ambiance.setVolume(QSettings().value("sound/ambiancevolume").toDouble());
	ambiance.setLoopCount(QSoundEffect::Infinite);
	ambiance.play();

	// IDLE
	idleTimer.start();
}

void MainWin::updateScene(BasicCamera& camera, QString const& pathId)
{
	if(!loaded)
	{
		return;
	}

	if(pathId == "cosmo")
	{
		if(QSettings().value("scripting/rootdir").toString() == "movie")
		{
			PythonQtHandler::evalScript("updateTimer("
			                            + QString::number(frameTiming) + ")");
		}
		if(cursorTimer.elapsed() > 2000
		   && cursor().shape() == Qt::CursorShape::ArrowCursor)
		{
			auto c(cursor());
			c.setShape(Qt::CursorShape::BlankCursor);
			setCursor(c);
		}
		if(networkManager->isServer())
		{
			animator->update(frameTiming, videomode);
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

		// handle idle mode
		if(idleTimer.isValid()
		   && (idleTimer.elapsed() / 1000.0)
		          >= QSettings().value("misc/idlemodewaittime").toDouble())
		{
			idleTimer.invalidate();
			animator->setIdleMode(true);
		}

		if(gamepadHandler.isEnabled())
		{
			auto multiplier
			    = QSettings().value("controls/rotationspeed").toDouble();
			auto rightJoystick(gamepadHandler.getJoystick(Side::RIGHT));
			float yaw(universe->getCamYaw()), pitch(universe->getCamPitch());
			yaw -= 2.0 * rightJoystick.x() * frameTiming * multiplier;
			pitch += 2.0 * rightJoystick.y() * frameTiming * multiplier;
			universe->setCamYaw(yaw);
			universe->setCamPitch(pitch);
			// handle idle mode
			if(rightJoystick != QVector2D{}
			   || gamepadHandler.getJoystick(Side::LEFT) != QVector2D{}
			   || gamepadHandler.getTrigger(Side::LEFT) != 0.0
			   || gamepadHandler.getTrigger(Side::RIGHT) != 0.0)
			{
				stopIdle();
			}

			if(movementControls->getGamepadVelocity().isNull()
			   && thrustSound.isPlaying())
			{
				thrustSound.stop();
			}
		}
		movementControls->update(
		    frameTiming, universe->isPlanetarySystemRendered(), gamepadHandler);

		if(networkManager->isServer())
		{
			timeController->update();
			animTimeSelect->update();
			tmController->update();
			scenes->update();
		}

		if((universe->getCosmoPosition() - Vector3(-0.43, -8.24, -0.81))
		       .length()
		   > 3.0e-3)
		{
			if(renderer.postProcessingPipeline.front().first == "lensing")
			{
				qDebug() << "removing";
				renderer.removePostProcessingShader("lensing");
			}
		}
		else if(renderer.postProcessingPipeline.front().first != "lensing")
		{
			qDebug() << "adding";
			renderer.insertPostProcessingShader("lensing", "lensing", 0);
		}
	}
	if(pathId == "planet")
	{
		auto& cam = dynamic_cast<OrbitalSystemCamera&>(camera);
		auto pos  = QSettings()
		               .value("misc/uilabelspos")
		               .value<QVector3D>(); //(-0.3f, 0.25f, -0.4f);
		pos *= QSettings().value("misc/uilabelsdistmul").toDouble();

		auto billboardPos = pos;
		billboardPos.setY(pos.y() + (0.06 * uiLabelsSizeMul));
		helperBillboard->width = 0.4f * uiLabelsSizeMul;

		if(vrHandler->isEnabled() && vrHandler->getDriverName() == "OpenVR")
		{
			helperBillboard->position = utils::transformPosition(
			    cam.hmdSpaceToWorldTransform(), billboardPos);

			debugText->getModel() = cam.hmdSpaceToWorldTransform();
			/*debugText->getModel().translate(pos);
			debugText->getModel().scale(
			    1.5 * static_cast<float>(textWidth) / width(),
			    1.5 * static_cast<float>(textHeight) / height());*/
		}
		else
		{
			helperBillboard->position = utils::transformPosition(
			    cam.cameraSpaceToWorldTransform(), billboardPos);

			debugText->getModel() = cam.cameraSpaceToWorldTransform();
		}
		auto scaleFactor = 4 * uiLabelsSizeMul;
		debugText->getModel().translate(pos);
		debugText->getModel().scale(
		    scaleFactor * static_cast<float>(textWidth) / width(),
		    scaleFactor * static_cast<float>(textWidth) / height());

		helperBillboard->getShader().setUniform("exposure",
		                                        toneMappingModel->exposure);
		helperBillboard->getShader().setUniform("dynamicrange",
		                                        toneMappingModel->dynamicrange);
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
		/*
		const std::string targetName(cam.target->getName());
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
				if(showHelperBillboard)
				{
					helperBillboard->render(camera);
				}
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
				if(showHelperBillboard)
				{
					helperBillboard->render(camera);
				}
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
			const QSizeF playAreaSize(vrHandler->getPlayAreaSize());
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
	auto const& cam(dynamic_cast<Camera const&>(camera));

	universe->renderCosmo(*toneMappingModel);

	// update here because depends on eye
	const QVector3D pos(
	    Utils::toQt(cam.dataToWorldPosition(-1 * Vector3(0.43, 8.24, 0.81))));
	lenseScreenCoord = camera.project(pos);
	lenseScreenCoord /= lenseScreenCoord.w();
	lenseDist
	    = (utils::transformPosition(camera.hmdScaledSpaceToWorldTransform(), {})
	       - pos)
	          .length();
}

void MainWin::renderGui(QSize const& targetSize, AdvancedPainter& painter)
{
	AbstractMainWin::renderGui(targetSize, painter);
	// will get disabled by QOpenGLPaintDevice anyway
	const GLStateSet glState({{GL_DEPTH_TEST, false}});
	if(!showInfoText)
	{
		return;
	}
	QString str(QString(PROJECT_NAME) + " - " + QString(PROJECT_VERSION)
	            + '\n');
	if(universe->isPlanetarySystemLoaded())
	{
		str += tr("Planetary system : ") + universe->getPlanetarySystemName()
		       + '\n';
		str += tr("Planet target : ") + universe->getPlanetTarget() + '\n';
		std::ostringstream stream;
		stream << universe->getPlanetPosition();
		str += tr("Planet rel. position (m) : ") + stream.str().c_str() + '\n';
	}
	std::ostringstream stream;
	stream << universe->getCosmoPosition();
	str += tr("Cosmology position (kpc) : ") + stream.str().c_str() + '\n';
	str += tr("Scale (real meter / sim meter) : ")
	       + QString::number(universe->getScale(), 'g', 5) + '\n';

	const QPen pen(Qt::red);
	painter.setPen(pen);
	painter.drawText(0, 0, targetSize.width(), targetSize.height(),
	                 Qt::AlignLeft | Qt::AlignTop, str);
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
			const QSize rtSize(vrHandler->getEyeRenderTargetSize());
			aspectRatio = rtSize.width();
			aspectRatio /= rtSize.height();
		}

		shader.setUniform("aspectRatio", aspectRatio);
		shader.setUniform("lenseSize",
		                  static_cast<float>(1.0e14 * universe->getScale()));
		shader.setUniform("lenseScreenCoord", lenseScreenCoord);
		shader.setUniform("lenseDist", lenseDist);
		shader.setUniform("radiusLimit", 0.2f);

		shader.setUniform("distortionMap", 2);
	}
}

std::vector<GLComputeShader::TextureBinding>
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
		return {
		    {*lenseDistortionMap, GLComputeShader::DataAccessMode::SAMPLER}};
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
		position = utils::transformPosition(
		    renderer.getCamera("cosmo").seatedTrackedSpaceToWorldTransform(),
		    cont->getPosition());
	}
	else
	{
		position = utils::transformPosition(
		    renderer.getCamera("cosmo").hmdScaledSpaceToWorldTransform(),
		    position);
	}

	// then data space
	position
	    = Utils::toQt(renderer.getCamera<Camera>("cosmo").worldToDataPosition(
	        Utils::fromQt(position)));
	QString posstr;
	posstr += QString::number(position.x());
	posstr += "; ";
	posstr += QString::number(position.y());
	posstr += "; ";
	posstr += QString::number(position.z());

	auto* msgBox = qt_owned<QMessageBox>();
	msgBox->setAttribute(Qt::WA_DeleteOnClose);
	msgBox->setStandardButtons(QMessageBox::Ok);
	msgBox->setWindowTitle(tr("Position selected"));
	msgBox->setText(posstr);
	msgBox->setModal(false);
	msgBox->show();
}
