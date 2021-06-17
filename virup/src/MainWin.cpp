#include "MainWin.hpp"

MainWin::MainWin()
{
	srand(time(nullptr));
}

QDateTime MainWin::getSimulationTime() const
{
	return SimulationTime::utToDateTime(clock.getCurrentUt());
}

void MainWin::setSimulationTime(QDateTime const& simulationTime)
{
	clock.setCurrentUt(SimulationTime::dateTimeToUT(simulationTime, false));
}

double MainWin::getScale() const
{
	return renderer.getCamera<Camera>("cosmo").scale * mtokpc;
}

void MainWin::setScale(double scale)
{
	renderer.getCamera<Camera>("cosmo").scale = scale / mtokpc;
	CelestialBodyRenderer::overridenScale     = scale;
}

Vector3 MainWin::getCosmoPosition() const
{
	return renderer.getCamera<Camera>("cosmo").position;
}

void MainWin::setCosmoPosition(Vector3 cosmoPosition)
{
	auto& cosmoCam(renderer.getCamera<Camera>("cosmo"));
	auto& planetCam(renderer.getCamera<OrbitalSystemCamera>("planet"));

	Vector3 diff(cosmoPosition - cosmoCam.position);
	cosmoCam.position = cosmoPosition;
	planetCam.relativePosition += diff / mtokpc;
}

QString MainWin::getPlanetTarget() const
{
	return universe->getPlanetTarget();
}

void MainWin::setPlanetTarget(QString const& name)
{
	universe->setPlanetTarget(name);
}

Vector3 MainWin::getPlanetPosition() const
{
	return renderer.getCamera<OrbitalSystemCamera>("planet").relativePosition;
}

void MainWin::setPlanetPosition(Vector3 planetPosition)
{
	auto& cosmoCam(renderer.getCamera<Camera>("cosmo"));
	auto& planetCam(renderer.getCamera<OrbitalSystemCamera>("planet"));

	Vector3 diff(planetPosition - planetCam.relativePosition);
	planetCam.relativePosition = planetPosition;
	cosmoCam.position += diff * mtokpc;
}

void MainWin::setCamPitch(float pitch)
{
	renderer.getCamera<Camera>("cosmo").pitch               = pitch;
	renderer.getCamera<OrbitalSystemCamera>("planet").pitch = pitch;
}

void MainWin::setCamYaw(float yaw)
{
	renderer.getCamera<Camera>("cosmo").yaw               = yaw;
	renderer.getCamera<OrbitalSystemCamera>("planet").yaw = yaw;
}

QString MainWin::getClosestCommonAncestorName(
    QString const& celestialBodyName0, QString const& celestialBodyName1) const
{
	return universe->getClosestCommonAncestorName(celestialBodyName0,
	                                              celestialBodyName1);
}

Vector3 MainWin::getCelestialBodyPosition(QString const& bodyName,
                                          QString const& referenceBodyName,
                                          QDateTime const& dt) const
{
	return universe->getCelestialBodyPosition(bodyName, referenceBodyName, dt,
	                                          clock.getCurrentUt());
}

Vector3 MainWin::interpolateCoordinates(QString const& celestialBodyName0,
                                        QString const& celestialBodyName1,
                                        float t) const
{
	return universe->interpolateCoordinates(
	    celestialBodyName0, celestialBodyName1, t, clock.getCurrentUt());
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
				float tc(clock.getTimeCoeff());
				if(tc > 1.f && !clock.getLockedRealTime())
				{
					clock.setTimeCoeff(tc / 10.f);
					debugText->setText(
					    ("Time coeff. : "
					     + std::to_string(static_cast<int>(tc / 10.f)) + "x")
					        .c_str());
					timeSinceTextUpdate = 0.f;
				}
			}
			else if(a.id == "timecoeffup")
			{
				float tc(clock.getTimeCoeff());
				if(tc < 1000000.f && !clock.getLockedRealTime())
				{
					clock.setTimeCoeff(tc * 10.f);
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
		if(dialog != nullptr)
		{
			dialog->close();
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
		QCursor::setPos(width() / 2, height() / 2);
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
		return;
	}
	float dx = (static_cast<float>(width()) / 2 - e->globalX()) / width();
	float dy = (static_cast<float>(height()) / 2 - e->globalY()) / height();
	auto& cam(renderer.getCamera<Camera>("cosmo"));
	cam.yaw += dx * 3.14f / 3.f;
	cam.pitch += dy * 3.14f / 3.f;
	auto& cam2(renderer.getCamera<OrbitalSystemCamera>("planet"));
	cam2.yaw += dx * 3.14f / 3.f;
	cam2.pitch += dy * 3.14f / 3.f;
	QCursor::setPos(width() / 2, height() / 2);
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
								float tc(clock.getTimeCoeff());
								if(padCoords[1] < 0.0f) // DOWN
								{
									if(tc > 1.f && !clock.getLockedRealTime())
									{
										clock.setTimeCoeff(tc / 10.f);
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
									   && !clock.getLockedRealTime())
									{
										clock.setTimeCoeff(tc * 10.f);
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
					case VRHandler::Button::TRIGGER:
						CelestialBodyRenderer::renderLabels
						    = CelestialBodyRenderer::renderLabels > 0.f ? 0.f
						                                                : 1.f;
						CelestialBodyRenderer::renderOrbits
						    = CelestialBodyRenderer::renderOrbits > 0.f ? 0.f
						                                                : 1.f;
						// toggleGrid();
						break;
					case VRHandler::Button::MENU:
						printPositionInDataSpace(e.side);
						break;
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

void MainWin::setupPythonAPI()
{
	PythonQtHandler::addObject("VIRUP", this);
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
	universe = new Universe(*camPlanet);

	// PLANETS LOADING
	debugText = new Text3D(textWidth, textHeight);
	debugText->setFlags(Qt::AlignCenter);
	debugText->setColor(QColor(255, 0, 0));
	debugText->setText("");

	movementControls = new MovementControls(
	    *vrHandler, universe->getBoundingBox(), cam, camPlanet);

	renderer.removeSceneRenderPath("default");

	renderer.appendSceneRenderPath("cosmo", Renderer::RenderPath(cam));
	renderer.appendSceneRenderPath("planet", Renderer::RenderPath(camPlanet));

	// we will draw them ourselves
	renderer.pathIdRenderingControllers = "";

	loaded = true;

	// LENSING
	lenseDistortionMap
	    = new GLTexture("data/virup/images/pointmass-distortion.png", false);

	renderer.appendPostProcessingShader("lensing", "lensing");

	// TRANSITIONS
	if(networkManager->isServer())
	{
		dialog = new QDialog;
		dialog->show();
		dialog->setWindowTitle("VIRUP Scenes");
		// dialog->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint |
		// Qt::X11BypassWindowManagerHint );

		auto layout = new QVBoxLayout(dialog);

		layout->addWidget(new QLabel("Scenes :"));

		QStringList scenes = {"0:International Space Station 1:1",
		                      "1:International Space Station 100:1",
		                      "2:Earth close-up",
		                      "3:Earth-Moon System",
		                      "4:Phobos",
		                      "5:Saturn Moons",
		                      "6:Rhea",
		                      "7:Solar System",
		                      "8:Kepler-11",
		                      "9:51 Peg",
		                      "AGORA",
		                      "IllustrisTNG",
		                      "SDSS close",
		                      "SDSS distant",
		                      "Gaia In",
		                      "Gaia Mid",
		                      "Gaia out"};
		for(int i(0); i < scenes.size(); ++i)
		{
			auto button = new QPushButton(scenes[i]);
			connect(button, &QPushButton::clicked, this, [i]() {
				PythonQtHandler::evalScript("setSceneId(" + QString::number(i)
				                            + ")");
			});
			button->setFocusPolicy(Qt::NoFocus);
			layout->addWidget(button);
			buttons.push_back(button);
		}
		layout->addWidget(new QLabel("Options :"));
		transitionsButton
		    = new QPushButton("Toggle transitions (only if user is sick, can "
		                      "introduce problems !)");
		connect(transitionsButton, &QPushButton::clicked, this,
		        []() { PythonQtHandler::evalScript("toggleAnimations()"); });
		transitionsButton->setFocusPolicy(Qt::NoFocus);
		layout->addWidget(transitionsButton);
	}
}

void MainWin::updateScene(BasicCamera& camera, QString const& pathId)
{
	if(!loaded)
	{
		return;
	}

	if(pathId == "cosmo")
	{
		auto& cam(dynamic_cast<Camera&>(camera));
		cam.currentFrameTiming = frameTiming;
		cam.updateTargetFPS();

		OctreeLOD::forceMaxQuality() = videomode;

		universe->updateCosmo(cam);

		movementControls->update(frameTiming,
		                         universe->isPlanetarySystemRendered());

		if(networkManager->isServer())
		{
			int currentScene(PythonQtHandler::getVariable("id").toInt());
			for(int i(0); i < static_cast<int>(buttons.size()); ++i)
			{
				auto button  = buttons[i];
				QPalette pal = button->palette();
				if(i == currentScene)
				{
					pal.setColor(QPalette::Button, QColor(Qt::green));
				}
				else
				{
					pal.setColor(QPalette::Button, QColor(255, 128, 128));
				}
				button->setAutoFillBackground(true);
				button->setPalette(pal);
				button->update();
			}
			QPalette pal = transitionsButton->palette();
			bool animationDisabled(
			    PythonQtHandler::getVariable("disableanimations").toBool());
			if(animationDisabled)
			{
				transitionsButton->setText("Transitions : DISABLED");
				pal.setColor(QPalette::Button, QColor(255, 128, 128));
			}
			else
			{
				transitionsButton->setText("Transitions : ENABLED");
				pal.setColor(QPalette::Button, QColor(Qt::green));
			}
			transitionsButton->setAutoFillBackground(true);
			transitionsButton->setPalette(pal);
			transitionsButton->update();
		}
	}
	if(pathId == "planet")
	{
		auto& cam      = dynamic_cast<OrbitalSystemCamera&>(camera);
		auto& cosmoCam = renderer.getCamera<Camera>("cosmo");
		if(vrHandler->isEnabled())
		{
			debugText->getModel() = cam.hmdSpaceToWorldTransform();
			debugText->getModel().translate(QVector3D(0.0f, -0.075f, -0.20f));
			debugText->getModel().scale(
			    1.5 * static_cast<float>(textWidth) / width(),
			    1.5 * static_cast<float>(textHeight) / height());
		}
		else
		{
			debugText->getModel() = cam.screenToWorldTransform();
			// debugText->getModel().translate(QVector3D(-0.88f, 0.88f,
			// 0.f));
			debugText->getModel().scale(
			    2 * static_cast<float>(textWidth) / width(),
			    2 * static_cast<float>(textWidth) / height());
		}
		debugText->getShader().setUniform("exposure",
		                                  toneMappingModel->exposure);
		debugText->getShader().setUniform("dynamicrange",
		                                  toneMappingModel->dynamicrange);
		if(videomode)
		{
			clock.update(frameTiming);
		}
		else
		{
			clock.update();
		}
		cam.updateUT(clock.getCurrentUt());

		if(!universe->planetSystems->renderSystem())
		{
			return;
		}
		universe->updatePlanetarySystem(cosmoCam, clock.getCurrentUt());

		timeSinceTextUpdate += frameTiming;
		std::string targetName(cam.target->getName());
		if(targetName != lastTargetName)
		{
			debugText->setText(QString("Locked to ") + targetName.c_str());
			timeSinceTextUpdate = 0.f;
			lastTargetName      = targetName;
		}
		return;
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
			if(timeSinceTextUpdate < 5.f)
			{
				// debugText->render();
			}
		}
		else
		{
			universe->renderPlanetarySystem();
			renderer.renderVRControls();
			universe->renderPlanetarySystemTransparent();
			if(timeSinceTextUpdate < 5.f)
			{
				// debugText->render();
			}
		}
		if(showGrid)
		{
			grid->render(getScale(), 1.125);
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

	universe->renderCosmo(cam, *toneMappingModel);

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
		shader.setUniform("lenseSize", static_cast<float>(1.0e14 * getScale()));
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
	delete dialog;
	delete lenseDistortionMap;
	delete debugText;
	delete movementControls;
	delete universe;
	delete grid;
}
