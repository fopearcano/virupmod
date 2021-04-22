#include "MainWin.hpp"

MainWin::MainWin()
{
	srand(time(nullptr));
}

void MainWin::loadNewSystem()
{
	delete systemRenderer;

	orbitalSystem  = planetSystems->getClosestSystem();
	systemRenderer = new OrbitalSystemRenderer(orbitalSystem);

	debugText->setText(QString(orbitalSystem->getName().c_str()));
	lastTargetName      = orbitalSystem->getMainCelestialBody()->getName();
	timeSinceTextUpdate = 0.f;

	auto barycenters = orbitalSystem->getAllBinariesNames();
	auto stars       = orbitalSystem->getAllStarsNames();
	auto fcPlanets   = orbitalSystem->getAllFirstClassPlanetsNames();
	auto satellites  = orbitalSystem->getAllSatellitePlanetsNames();

	std::cout << "-=-=- SYSTEM " << orbitalSystem->getName() << " -=-=-"
	          << std::endl;
	std::cout << "Barycenters : " << barycenters.size() << std::endl;
	for(auto const& name : barycenters)
	{
		std::cout << name << std::endl;
	}
	std::cout << std::endl;

	std::cout << "Stars : " << stars.size() << std::endl;
	for(auto const& name : stars)
	{
		std::cout << name << std::endl;
	}
	std::cout << std::endl;

	std::cout << "Main Planets : " << fcPlanets.size() << std::endl;
	for(auto const& name : fcPlanets)
	{
		std::cout << name << std::endl;
	}
	std::cout << std::endl;

	std::cout << "Satellites : " << satellites.size() << std::endl;
	for(auto const& name : satellites)
	{
		std::cout << name << "("
		          << (*orbitalSystem)[name]->getParent()->getName() << ")"
		          << std::endl;
	}
	std::cout << std::endl;

	if(camPlanet == nullptr)
	{
		camPlanet
		    = new OrbitalSystemCamera(*vrHandler, toneMappingModel->exposure,
		                              toneMappingModel->dynamicrange);
		camPlanet->seatedVROrigin = false;
		camPlanet->setPerspectiveProj(renderer.getVerticalFOV(),
		                              renderer.getAspectRatioFromFOV());
	}
	camPlanet->target           = orbitalSystem->getMainCelestialBody();
	camPlanet->relativePosition = Vector3(
	    camPlanet->target->getCelestialBodyParameters().radius * 2.0, 0.0, 0.0);

	CelestialBodyRenderer::overridenScale = 1.0;
	forceUpdateFromCosmo                  = true;

	planetarySystemName = orbitalSystem->getName().c_str();
}

QDateTime MainWin::getSimulationTime() const
{
	return SimulationTime::utToDateTime(clock.getCurrentUt());
}

void MainWin::setSimulationTime(QDateTime const& simulationTime)
{
	clock.setCurrentUt(SimulationTime::dateTimeToUT(simulationTime, false));
}

float MainWin::getCosmoLum() const
{
	return cosmologicalSim->brightnessMultiplier;
}

void MainWin::setCosmoLum(float cosmoLum)
{
	cosmologicalSim->brightnessMultiplier = cosmoLum;
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
	return renderer.getCamera<OrbitalSystemCamera>("planet")
	    .target->getName()
	    .c_str();
}

void MainWin::setPlanetTarget(QString const& name)
{
	auto ptrs = orbitalSystem->getAllCelestialBodiesPointers();
	for(auto ptr : ptrs)
	{
		if(QString(ptr->getName().c_str()) == name)
		{
			renderer.getCamera<OrbitalSystemCamera>("planet").target = ptr;
		}
	}
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
	Orbitable const* orb0(nullptr);
	Orbitable const* orb1(nullptr);

	auto ptrs = orbitalSystem->getAllCelestialBodiesPointers();
	for(auto ptr : ptrs)
	{
		if(QString(ptr->getName().c_str()) == celestialBodyName0)
		{
			orb0 = ptr;
		}
		if(QString(ptr->getName().c_str()) == celestialBodyName1)
		{
			orb1 = ptr;
		}
	}
	if(orb0 == nullptr || orb1 == nullptr)
	{
		return "";
	}
	auto result(Orbitable::getCommonAncestor(orb0, orb1));
	if(result == nullptr)
	{
		return "";
	}
	return result->getName().c_str();
}

Vector3 MainWin::getCelestialBodyPosition(QString const& bodyName,
                                          QString const& referenceBodyName,
                                          QDateTime const& dt) const
{
	Orbitable const* orb(nullptr);
	Orbitable const* orbRef(nullptr);

	auto ptrs = orbitalSystem->getAllCelestialBodiesPointers();
	for(auto ptr : ptrs)
	{
		if(QString(ptr->getName().c_str()) == bodyName)
		{
			orb = ptr;
		}
		if(QString(ptr->getName().c_str()) == referenceBodyName)
		{
			orbRef = ptr;
		}
	}
	if(orb == nullptr || orbRef == nullptr)
	{
		return {};
	}
	if(dt.isValid())
	{
		return Orbitable::getRelativePositionAtUt(
		    orbRef, orb, SimulationTime::dateTimeToUT(dt));
	}
	return Orbitable::getRelativePositionAtUt(orbRef, orb,
	                                          clock.getCurrentUt());
}

Vector3 MainWin::interpolateCoordinates(QString const& celestialBodyName0,
                                        QString const& celestialBodyName1,
                                        float t) const
{
	Orbitable const* orb0(nullptr);
	Orbitable const* orb1(nullptr);

	auto ptrs = orbitalSystem->getAllCelestialBodiesPointers();
	for(auto ptr : ptrs)
	{
		if(QString(ptr->getName().c_str()) == celestialBodyName0)
		{
			orb0 = ptr;
		}
		if(QString(ptr->getName().c_str()) == celestialBodyName1)
		{
			orb1 = ptr;
		}
	}
	if(orb0 == nullptr || orb1 == nullptr)
	{
		return {};
	}
	auto ancestor(Orbitable::getCommonAncestor(orb0, orb1));
	if(ancestor == nullptr)
	{
		return {};
	}

	return (Orbitable::getRelativePositionAtUt(ancestor, orb0,
	                                           clock.getCurrentUt())
	        * (1 - t))
	       + (Orbitable::getRelativePositionAtUt(ancestor, orb1,
	                                             clock.getCurrentUt())
	          * t);
}

void MainWin::actionEvent(BaseInputManager::Action a, bool pressed)
{
	if(loaded)
	{
		if(pressed)
		{
			if(a.id == "alphaup")
			{
				setCosmoLum(getCosmoLum() * 10 / 8);
			}
			else if(a.id == "alphadown")
			{
				setCosmoLum(getCosmoLum() * 8 / 10);
			}
			else if(a.id == "resetvrpos")
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
				cosmologicalSim->trees.toggleDarkMatter();
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
									setCosmoLum(getCosmoLum() * 8 / 10);
								}
								else // RIGHT
								{
									setCosmoLum(getCosmoLum() * 10 / 8);
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
		    planetSystems->renderSystem());
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

	// COSMO LOADING
	cosmologicalSim = new CosmologicalSimulation(
	    QSettings().value("data/gazfile").toString().toStdString(),
	    QSettings().value("data/starsfile").toString().toStdString(),
	    QSettings().value("data/loaddarkmatter").toBool()
	        ? QSettings().value("data/darkmatterfile").toString().toStdString()
	        : "");
	cosmologicalSim->referenceFrame = UniverseElement::ReferenceFrame::GALACTIC;
	cosmologicalSim->unit           = 1.0;
	cosmologicalSim->solarsystemPosition = Vector3(-8.29995608, 0.0, 0.027);

	hyg       = new CSVObjects(QSettings().value("data/hyg").toString(),
                         QSettings().value("data/hygcon").toString());
	hyg->unit = 0.001;

	sdss = new CSVObjects(QSettings().value("data/sdss").toString(), true);
	sdss->unit                 = 1000.0;
	sdss->brightnessMultiplier = 1e9;

	planetSystems = new PlanetarySystems;

	// PLANETS LOADING
	debugText = new Text3D(textWidth, textHeight);
	debugText->setFlags(Qt::AlignCenter);
	debugText->setColor(QColor(255, 0, 0));

	loadNewSystem();
	orbitalSystem  = planetSystems->getClosestSystem();
	systemRenderer = new OrbitalSystemRenderer(orbitalSystem);

	debugText->setText("");

	movementControls = new MovementControls(
	    *vrHandler, cosmologicalSim->getBoundingBox(), cam, camPlanet);

	setCosmoLum(getCosmoLum() / (cam->scale * cam->scale));

	renderer.removeSceneRenderPath("default");

	renderer.appendSceneRenderPath("cosmo", Renderer::RenderPath(cam));
	renderer.appendSceneRenderPath("planet", Renderer::RenderPath(camPlanet));

	// we will draw them ourselves
	renderer.pathIdRenderingControllers = "";

	loaded = true;

	// LABELS
	QString labelspath(QSettings().value("data/cosmolabelsfile").toString());
	if(labelspath != "")
	{
		QFile f(labelspath);
		if(!f.open(QFile::ReadOnly | QFile::Text))
		{
			std::cerr << "Invalid cosmological labels file path : "
			          << labelspath.toStdString() << std::endl;
		}
		else
		{
			QTextStream in(&f);
			while(!in.atEnd())
			{
				QString line       = in.readLine();
				QStringList fields = line.split(",");
				QString label(fields[0]);
				Vector3 dataPos(fields[1].toDouble(), fields[2].toDouble(),
				                fields[3].toDouble());

				dataPos = Utils::fromQt(cosmologicalSim->getRelToAbsTransform()
				                        * Utils::toQt(-1.0 * dataPos));

				auto labelText = new LabelRenderer(label, QColor(255, 0, 0));
				cosmoLabels.emplace_back(dataPos, labelText);
			}
		}
	}

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

		QStringList scenes = {"0:International Space Station",
		                      "1:Earth close-up",
		                      "2:Earth-Moon System",
		                      "3:Phobos",
		                      "4:Saturn Moons",
		                      "5:Rhea",
		                      "6:Solar System",
		                      "7:Kepler-11",
		                      "8:51 Peg",
		                      "9:TRAPPIST-1",
		                      ":SWEEPS-11",
		                      ":Milky Way",
		                      ":SDSS"};
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

		planetSystems->update(cam);
		planetSystems->useVRCamposForClosest
		    = PythonQtHandler::getVariable("id").toInt() == -1;

		Vector3 camPosData(cam.worldToDataPosition(Utils::fromQt(
		    cam.hmdScaledSpaceToWorldTransform() * QVector3D(0.f, 0.f, 0.f))));

		for(auto cosmoLabel : cosmoLabels)
		{
			Vector3 pos(cam.dataToWorldPosition(cosmoLabel.first));
			Vector3 camRelPos(camPosData - cosmoLabel.first);
			Vector3 unitRelPos(camRelPos.getUnitForm());

			float yaw(atan2(unitRelPos[1], unitRelPos[0]));
			float pitch(-1.0 * asin(unitRelPos[2]));
			double rescale(pos.length() <= 8000.0 ? 1.0
			                                      : 8000.0 / pos.length());
			QMatrix4x4 model;
			model.translate(Utils::toQt(pos * rescale));
			model.scale(rescale * camRelPos.length() * cam.scale / 3.0);
			model.rotate(yaw * 180.f / M_PI + 90.f, 0.0, 0.0, 1.0);
			model.rotate(pitch * 180.f / M_PI + 90.f, 1.0, 0.0, 0.0);
			cosmoLabel.second->updateModel(model);
		}
		movementControls->update(frameTiming, planetSystems->renderSystem());

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

		timeSinceTextUpdate += frameTiming;
		if(!planetSystems->renderSystem())
		{
			return;
		}
		if(lastData != planetSystems->getClosestSystemPosition())
		{
			planetarySystemName = "";
			loadNewSystem();
		}
		else if(planetarySystemName != orbitalSystem->getName().c_str())
		{
			loadNewSystem();
		}

		lastData   = planetSystems->getClosestSystemPosition();
		sysInWorld = cosmoCam.dataToWorldPosition(lastData);

		CelestialBodyRenderer::overridenScale = mtokpc * cosmoCam.scale;

		if((cam.target == orbitalSystem->getMainCelestialBody()
		    && CelestialBodyRenderer::overridenScale < 1e-12)
		   || forceUpdateFromCosmo)
		{
			cam.relativePosition = -1 * sysInWorld / (mtokpc * cosmoCam.scale);
			forceUpdateFromCosmo = false;
		}

		sysInWorld = cosmoCam.dataToWorldPosition(lastData);

		clock.update();
		cam.updateUT(clock.getCurrentUt());

		systemRenderer->updateMesh(clock.getCurrentUt(), cam);

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
		if(!planetSystems->renderSystem())
		{
			if(timeSinceTextUpdate < 5.f)
			{
				// debugText->render();
			}
		}
		else
		{
			systemRenderer->render(camera);
			renderer.renderVRControls();
			systemRenderer->renderTransparent(camera);
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

	if(!planetSystems->renderSystem())
	{
		renderer.renderVRControls();
	}
	auto& cam(dynamic_cast<Camera const&>(camera));

	GLHandler::glf().glDepthFunc(GL_LEQUAL);
	GLHandler::glf().glEnable(GL_DEPTH_CLAMP);
	GLHandler::glf().glEnable(GL_CLIP_DISTANCE0);
	hyg->constellationsLabels = CelestialBodyRenderer::renderLabels;
	hyg->constellationsAlpha  = CelestialBodyRenderer::renderLabels;
	hyg->render(cam, toneMappingModel);
	sdss->render(cam, toneMappingModel);
	planetSystems->render(cam, toneMappingModel);
	cosmologicalSim->render(cam, toneMappingModel);

	// TODO(florian) better than this
	if(CelestialBodyRenderer::renderLabels > 0.f)
	{
		for(auto cosmoLabel : cosmoLabels)
		{
			if(cosmoLabel.first == solarSystemDataPos
			   && planetSystems->renderSystem()
			   && orbitalSystem->getName() == "Solar System")
			{
				continue;
			}
			cosmoLabel.second->render(toneMappingModel->exposure,
			                          toneMappingModel->dynamicrange);
		}
	}
	GLHandler::glf().glDisable(GL_CLIP_DISTANCE0);
	GLHandler::glf().glDisable(GL_DEPTH_CLAMP);

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
	for(auto cosmoLabel : cosmoLabels)
	{
		delete cosmoLabel.second;
	}
	delete systemRenderer;
	delete debugText;
	delete movementControls;
	delete planetSystems;
	delete sdss;
	delete hyg;
	delete cosmologicalSim;
	delete grid;
}
