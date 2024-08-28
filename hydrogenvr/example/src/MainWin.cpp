#include "Logger.hpp"
#include "MainWin.hpp"

#include <algorithm>

void MainWin::actionEvent(BaseInputManager::Action const& a, bool pressed)
{
	if(!pressed)
	{
		AbstractMainWin::actionEvent(a, pressed);
		return;
	}

	if(a.id == "barrelup")
	{
		barrelPower = 1.f + (barrelPower - 1.f) * 1.2f;
	}
	else if(a.id == "barreldown")
	{
		barrelPower = 1.f + (barrelPower - 1.f) / 1.2f;
	}
	else if(a.id == "togglevrorigin")
	{
		renderer.getCamera("default").seatedVROrigin
		    = !renderer.getCamera("default").seatedVROrigin;
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

		QSize windowSize(this->size());
		windowSize *= this->screen()->devicePixelRatio();

		QCursor::setPos(this->x() + windowSize.width() / 2,
		                this->y() + windowSize.height() / 2);
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
	if(!isActive() || /*vrHandler->isEnabled() ||*/ !moveView)
	{
		return;
	}
	QSize windowSize(this->size());
	windowSize *= this->screen()->devicePixelRatio();
	if(QSettings().value("misc/mouseview").toBool())
	{
		float dx = (this->x() + static_cast<float>(windowSize.width()) / 2
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		            - e->globalPosition().x())
#else
		            - e->globalX())
#endif
		           / width();
		float dy = (this->y() + static_cast<float>(windowSize.height()) / 2
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		            - e->globalPosition().y())
#else
		            - e->globalY())
#endif
		           / height();
		yaw += dx * 3.14f / 3.f;
		pitch += dy * 3.14f / 3.f;
	}
	QCursor::setPos(this->x() + windowSize.width() / 2,
	                this->y() + windowSize.height() / 2);
}

void MainWin::gamepadEvent(GamepadHandler::Event const& e)
{
	if(e.type == GamepadHandler::EventType::BUTTON_PRESSED)
	{
		switch(e.button)
		{
			case GamepadHandler::Button::UP:
				if(toneMappingModel->autoexposure)
				{
					toneMappingModel->autoexposurecoeff *= 1.5f;
				}
				else
				{
					toneMappingModel->exposure *= 1.5f;
				}
				break;
			case GamepadHandler::Button::DOWN:
				if(toneMappingModel->autoexposure)
				{
					toneMappingModel->autoexposurecoeff /= 1.5f;
				}
				else
				{
					toneMappingModel->exposure /= 1.5f;
				}
				break;
			case GamepadHandler::Button::LEFT:
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
				break;
			case GamepadHandler::Button::RIGHT:
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
				break;
			default:
				break;
		}
	}

	AbstractMainWin::gamepadEvent(e);
}

void MainWin::initScene()
{
	qDebug() << "This will be logged formatted and anotated.";
	{
		Logger::NoFormatGuard nfg;
		qDebug() << "This will be";
		qDebug() << " logged raw.\n";
	}
	qDebug() << "The guard restores logging state after the scope.";

	// SKYBOX
	sbShader.load("skybox");
	skybox = std::make_unique<GLMesh>();
	Primitives::setAsUnitCube(*skybox, sbShader);

	std::array<QString, 6> paths = {};
	paths.at(static_cast<unsigned int>(GLTexture::CubemapFace::BACK))
	    = "data/example/images/ame_ash/ashcanyon_bk.png";
	paths.at(static_cast<unsigned int>(GLTexture::CubemapFace::BOTTOM))
	    = "data/example/images/ame_ash/ashcanyon_dn.png";
	paths.at(static_cast<unsigned int>(GLTexture::CubemapFace::FRONT))
	    = "data/example/images/ame_ash/ashcanyon_ft.png";
	paths.at(static_cast<unsigned int>(GLTexture::CubemapFace::LEFT))
	    = "data/example/images/ame_ash/ashcanyon_lf.png";
	paths.at(static_cast<unsigned int>(GLTexture::CubemapFace::RIGHT))
	    = "data/example/images/ame_ash/ashcanyon_rt.png";
	paths.at(static_cast<unsigned int>(GLTexture::CubemapFace::TOP))
	    = "data/example/images/ame_ash/ashcanyon_up.png";
	sbTexture = std::make_unique<GLTexture>(paths);

	shaderProgram.load("colorpervert");
	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------

	std::vector<float> vertices = {
	    0.5f,  0.5f,  0.0f, // top right
	    0.5f,  -0.5f, 0.0f, // bottom right
	    -0.5f, -0.5f, 0.0f, // bottom left
	    -0.5f, 0.5f,  0.0f, // top left
	};
	/*std::vector<float> vertices = {
	    0.5f,  0.5f,  0.0f, 1.f, 0.f, 0.f, // top right
	    0.5f,  -0.5f, 0.0f, 0.f, 1.f, 0.f, // bottom right
	    -0.5f, -0.5f, 0.0f, 0.f, 0.f, 1.f, // bottom left
	    -0.5f, 0.5f,  0.0f, 0.f, 0.f, 0.f, // top left
	};*/
	std::vector<unsigned int> indices = {
	    // note that we start from 0!
	    0, 1, 3, // first Triangle
	    1, 2, 3  // second Triangle
	};
	mesh = std::make_unique<GLMesh>();
	mesh->setVertexShaderMapping(shaderProgram, {{"position", 3}});
	mesh->setVertices(vertices, indices);
	shaderProgram.setUnusedAttributesValues({{"color", {1.0, 1.0, 0.0}}});

	// create cube
	movingCube = std::make_unique<MovingCube>();

	// create points
	pointsMesh = std::make_unique<GLMesh>();
	pointsShader.load("default");
	pointsShader.setUniform("alpha", 1.0f);
	pointsShader.setUniform("color", QColor::fromRgbF(1.0f, 1.0f, 1.0f));
	std::vector<float> points = {0, 0, 0};
	pointsMesh->setVertexShaderMapping(pointsShader, {{"position", 3}});
	pointsMesh->setVertices(points);

	sphereShader.load("default");
	sphereShader.setUniform("alpha", 1.0f);
	sphereShader.setUniform("color", QColor::fromRgbF(0.5f, 0.5f, 1.0f));
	sphere = std::make_unique<GLMesh>();
	Primitives::setAsUnitSphere(*sphere, sphereShader, 100, 100);

	playareaShader.load("default");
	playareaShader.setUniform("color", QColor(255, 0, 0));
	playareaShader.setUniform("alpha", 1.f);
	playarea = std::make_unique<GLMesh>();
	if(vrHandler->isEnabled())
	{
		auto playareaquad(vrHandler->getPlayAreaQuad());
		vertices = {
		    playareaquad[0].x(), playareaquad[0].y(), playareaquad[0].z(),
		    playareaquad[1].x(), playareaquad[1].y(), playareaquad[1].z(),
		    playareaquad[2].x(), playareaquad[2].y(), playareaquad[2].z(),
		    playareaquad[3].x(), playareaquad[3].y(), playareaquad[3].z(),
		};
		indices = {0, 1, 1, 2, 2, 3, 3, 0};
		playarea->setVertexShaderMapping(playareaShader, {{"position", 3}});
		playarea->setVertices(vertices, indices);
	}

	model                 = std::make_unique<Model>("models/drone/scene.gltf");
	light0                = std::make_unique<Light>();
	light0->ambiantFactor = 0.05f;
	light1                = std::make_unique<Light>();
	light1->ambiantFactor = 0.05f;

	GLTexture tex("data/example/images/cc.ktx");
	tex.setName("cc.ktx");
	tex.setSampler(GLTexture::Sampler(GL_NEAREST, GL_CLAMP_TO_EDGE));
	bill           = std::make_unique<Billboard>(std::move(tex));
	bill->position = QVector3D(0.f, 0.f, 0.8f);

	text = std::make_unique<Text3D>(200, 40);
	text->setColor(QColor(0, 0, 0, 255));
	text->setBackgroundColor(QColor(255, 0, 0, 127));
	text->setRectangle(QRect(50, 0, 150, 40));
	text->setSuperSampling(2.f);
	text->setFlags(Qt::AlignRight);

	text->getModel().rotate(135.f, 0.f, 0.f, 1.f);
	text->getModel().rotate(45.f, 1.f, 0.f);
	text->getModel().translate(-0.6f, 0.f, 0.5f);

	widget3d = std::make_unique<Widget3D>(calendar);

	widget3d->getModel().rotate(135.f, 0.f, 0.f, 1.f);
	widget3d->getModel().rotate(45.f, 1.f, 0.f);
	widget3d->getModel().translate(0.6f, 0.f, 0.5f);

	dialog = std::make_unique<DemoDialog>();
	dialog3dWheel->addDialog3D("Demo Dialog", *dialog);

	volume
	    = std::make_unique<Volume>(GLTexture("data/example/images/volume.ktx"));
	image = QImage("data/example/images/cc.png");

	/*
	largeGridShader = std::make_unique<GLShaderProgram>("grid");
	largeGridTex = std::make_unique<GLTexture>(GLTexture::Tex2DProperties(1000,
	1000, GL_RGBA8)); GLComputeShader cmp("grid.comp");
	cmp.exec({{largeGridTex.get(), GLComputeShader::W}}, {1000, 1000, 1});
	largeGridTex->generateMipmap();

	largeGridMesh = std::make_unique<GLMesh>();
	Primitives::setAsQuad(*largeGridMesh, *largeGridShader,
	PrimitiveType::TRIANGLES);*/

	auto tools(menuBar->addMenu(tr("Tools")));
	tools->addAction(tr("Demo Dialog"), this,
	                 [this]() { this->dialog->show(); });

	renderer.getCamera("default").setEyeDistanceFactor(1.0f);

	renderer.appendPostProcessingShader("distort", "distort");
	renderer.appendPostProcessingShader("noisepostproc", "noisepostproc");
	renderer.renderControllersBeforeScene = false;

	timer.start();
}

void MainWin::updateScene(BasicCamera& camera, QString const& /*pathId*/)
{
	auto fps = std::clamp(1.f / frameTiming, 0.f, 120.f);
	text->setText(tr("Hello World !\n") + QString::number(round(fps)) + " FPS");
	if(gamepadHandler.isEnabled())
	{
		auto leftJoystick(gamepadHandler.getJoystick(Side::LEFT));
		auto rightJoystick(gamepadHandler.getJoystick(Side::RIGHT));
		yaw -= 2.0 * rightJoystick.x() * frameTiming;
		pitch += 2.0 * rightJoystick.y() * frameTiming;

		QVector3D lookDir(-cosf(yaw) * cosf(pitch), -sinf(yaw) * cosf(pitch),
		                  sinf(pitch));
		QVector3D up(0.0, 0.0, 1.0);
		QVector3D left(QVector3D::crossProduct(up, lookDir));
		up = QVector3D::crossProduct(lookDir, left);

		campos -= 2.0 * leftJoystick.x() * left * frameTiming;
		campos -= 2.0 * leftJoystick.y() * lookDir * frameTiming;

		campos
		    -= 2.0 * gamepadHandler.getTrigger(Side::LEFT) * up * frameTiming;
		campos
		    += 2.0 * gamepadHandler.getTrigger(Side::RIGHT) * up * frameTiming;
	}
	QVector3D lookDir(-cosf(yaw) * cosf(pitch), -sinf(yaw) * cosf(pitch),
	                  sinf(pitch));

	camera.setView(campos, lookDir, {0, 0, 1});

	Controller const* cont(vrHandler->getController(Side::LEFT));
	if(cont == nullptr)
	{
		cont = vrHandler->getController(Side::RIGHT);
	}
	if(cont != nullptr)
	{
		if(cont->getTriggerValue() > 0.5)
		{
			QVector4D pos(camera.seatedTrackedSpaceToWorldTransform()
			              * QVector4D(cont->getPosition(), 1.f));
			std::vector<float> points(3);
			points[0] = pos[0];
			points[1] = pos[1];
			points[2] = pos[2];
			pointsMesh->setVertices(points);
		}
	}

	Hand const* leftHand(vrHandler->getHand(Side::LEFT));
	if(leftHand != nullptr)
	{
		if(leftHand->isClosed())
		{
			QVector4D pos(camera.hmdSpaceToWorldTransform()
			              * QVector4D(leftHand->palmPosition(), 1.f));
			std::vector<float> points(3);
			points[0] = pos[0];
			points[1] = pos[1];
			points[2] = pos[2];
			pointsMesh->setVertices(points);
		}
	}

	movingCube->update();

	modelModel = QMatrix4x4();
	modelModel.scale(1.5 / model->getBoundingSphere().radius);
	float secs(timer.elapsed() / 5000.f);
	light0->setDirection(QVector3D(cos(secs / 4.0), sin(secs / 4.0), 0.0));
	light0->color = {0.f, 1.f, 0.f};
	/*= QVector3D(128 + 127 * cos(secs / 2.0), 128 + 127 * sin(secs / 2.0), 0)
	  / 255.f;*/
	light1->color = QVector3D(255, 255, 255) / 255.f;
	if(vrHandler->isEnabled())
	{
		QVector3D t(0.f, 1.4f * model->getBoundingSphere().radius, 0.f);
		modelModel.translate(t);
		modelModel.rotate(180.f, QVector3D(0.f, 1.f, 0.f));
		// light->direction = QVector3D(cos(secs), 0.f, sin(secs));
		light0->setBoundingSphereRadius(1.5);
		light1->setBoundingSphereRadius(1.5);
		light0->setCenter(t * 1.5 / model->getBoundingSphere().radius);
		light1->setCenter(t * 1.5 / model->getBoundingSphere().radius);
	}
	else
	{
		QVector3D t(0.f, 0.f, -50.f);
		modelModel.translate(t);
		modelModel.rotate(180.f, QVector3D(0.f, 0.f, 1.f));
		modelModel.rotate(120.f, QVector3D(1.f, 1.f, 1.f).normalized());
		modelModel.scale(0.3);
		// light->direction = QVector3D(sin(secs), cos(secs), 0.f);
		light0->setBoundingSphereRadius(1.5 * 0.3);
		light1->setBoundingSphereRadius(1.5 * 0.3);
		light0->setCenter(t * 1.5 / model->getBoundingSphere().radius);
		light1->setCenter(t * 1.5 / model->getBoundingSphere().radius);
	}
	modelModel.rotate(100.f * secs, QVector3D(0.f, 1.f, 0.f));
	model->generateShadowMap(modelModel, {light0.get(), light1.get()});
}

void MainWin::renderScene(BasicCamera const& camera, QString const& /*pathId*/)
{
	QMatrix4x4 skyboxSize;
	skyboxSize.scale(1000.f);
	GLHandler::useTextures({sbTexture.get()});
	{
		GLStateSet glState({{GL_CULL_FACE, false}});
		GLHandler::setUpRender(sbShader, skyboxSize,
		                       GLHandler::GeometricSpace::SKYBOX);
		skybox->render(PrimitiveType::TRIANGLE_STRIP);
	}
	GLHandler::clearDepthBuffer();

	QMatrix4x4 modelSphere;
	modelSphere.translate(-1.5, 0, 0);
	GLHandler::setUpRender(sphereShader, modelSphere,
	                       GLHandler::GeometricSpace::SKYBOX);
	sphere->render();
	GLHandler::clearDepthBuffer();

	movingCube->render();

	GLHandler::setUpRender(shaderProgram);
	mesh->render();
	GLHandler::setUpRender(pointsShader);
	GLHandler::setPointSize(8);
	pointsMesh->render();
	GLHandler::setPointSize(1);

	GLHandler::setUpRender(playareaShader, QMatrix4x4(),
	                       GLHandler::GeometricSpace::STANDINGTRACKED);
	playarea->render(PrimitiveType::LINES);

	if(vrHandler->isEnabled())
	{
		model->render(/*camera.standingTrackedSpaceToWorldTransform().inverted()
		                  * */camera/*.getWorldSpacePosition()*/,
		              modelModel,
		              std::vector<const Light*>{light0.get(), light1.get()}/*,
		              GLHandler::GeometricSpace::STANDINGTRACKED*/);
	}
	else
	{
		model->render(camera, modelModel,
		              std::vector<const Light*>{light0.get(), light1.get()});
	}

	widget3d->render(*toneMappingModel);
	bill->render(camera);
	text->render();

	volume->render(camera);

	/*
	QMatrix4x4 model;
	model.scale(100.f);
	GLHandler::useTextures({largeGridTex.get()});
	GLHandler::setUpRender(*largeGridShader, model);
	largeGridMesh->render();*/
}

void MainWin::renderGui(QSize const& targetSize, AdvancedPainter& painter)
{
	painter.setPen(QPen{Qt::red});
	painter.drawText(0, 0, targetSize.width(), targetSize.height(),
	                 Qt::AlignRight | Qt::AlignBottom,
	                 QString(PROJECT_NAME) + " - " + QString(PROJECT_VERSION));
	painter.setPen(QPen{Qt::green});
	painter.drawRects(QVector<QRect>{{0, 0, 64, 64}, {64, 64, 64, 64}});
	QRectF target(256.0, 0.0, image.width(), image.height());
	QRectF source(image.width() * 0.5, 0.0, image.width() * 0.5,
	              image.height() * 0.5);
	painter.drawImage(target, image, source);

	painter.setPen(QPen{Qt::blue});
	QRectF r{128, 128, 128, 128};
	painter.drawEllipse(r);

	AbstractMainWin::renderGui(targetSize, painter);
}

void MainWin::applyPostProcShaderParams(
    QString const& id, GLShaderProgram const& shader,
    GLFramebufferObject const& currentTarget) const
{
	AbstractMainWin::applyPostProcShaderParams(id, shader, currentTarget);
	if(id == "distort")
	{
		shader.setUniform("BarrelPower", barrelPower);
	}
	else if(id == "noisepostproc")
	{
		shader.setUniform("seed", static_cast<unsigned int>(timer.elapsed()));
	}
}
