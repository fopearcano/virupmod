#ifndef MAINWIN_H
#define MAINWIN_H

#include <QCoreApplication>
#include <QDirIterator>
#include <QJsonDocument>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSound>
#include <QSoundEffect>
#include <QVBoxLayout>

#include "AbstractMainWin.hpp"
#include "Text3D.hpp"

#include "Grid.hpp"
#include "MovementControls.hpp"
#include "scenes/Animator.hpp"
#include "ui/AnimationTimeSelector.hpp"
#include "ui/PlanetarySystemSelector.hpp"
#include "ui/PresenterHelp.hpp"
#include "ui/SceneSelector.hpp"
#include "ui/TimeController.hpp"
#include "ui/ToneMappingController.hpp"
#include "ui/UniverseElementSelector.hpp"
#include "ui/Visibilities.hpp"
#include "universe/Universe.hpp"

#include "graphics/OrbitalSystemCamera.hpp"
#include "graphics/renderers/OrbitalSystemRenderer.hpp"
#include "physics/OrbitalSystem.hpp"
#include "physics/SimulationTime.hpp"

#include "LibPlanet.hpp"

/** @ingroup pycall
 *
 * Callable in Python as the "VIRUP" object.
 */
class MainWin : public AbstractMainWin
{
	Q_OBJECT
	/**
	 * @brief Wether the orbits is enabled or not.
	 *
	 * @accessors renderOrbits(), setRenderOrbits()
	 */
	Q_PROPERTY(float renderOrbits READ renderOrbits WRITE setRenderOrbits)
	/**
	 * @brief Wether the labels is enabled or not.
	 *
	 * @accessors renderLabels(), setRenderLabels()
	 */
	Q_PROPERTY(float renderLabels READ renderLabels WRITE setRenderLabels)
	/**
	 * @brief Wether the darkmatter is enabled or not.
	 *
	 * @accessors darkmatterEnabled(), setDarkmatterEnabled()
	 */
	Q_PROPERTY(bool darkmatterEnabled READ darkmatterEnabled WRITE
	               setDarkmatterEnabled)
	/**
	 * @brief Wether the grid is enabled or not.
	 *
	 * @accessors gridEnabled(), setGridEnabled()
	 */
	Q_PROPERTY(bool gridEnabled READ gridEnabled WRITE setGridEnabled)
	Q_PROPERTY(bool isServer READ isServer)

	bool isServer() const { return networkManager->isServer(); };

  public:
	class State : public AbstractState
	{
	  public:
		State()                   = default;
		State(State const& other) = default;
		State(State&& other)      = default;
		virtual void readFromDataStream(QDataStream& stream) override
		{
			toneMappingState.readFromDataStream(stream);
			cosmoCamState.readFromDataStream(stream);
			planetCamState.readFromDataStream(stream);
			stream >> renderLabels;
			stream >> renderOrbits;
			stream >> compass;
			compassState.readFromDataStream(stream);
			stream >> stereoMultiplier;
			universeState.readFromDataStream(stream);
		};
		virtual void writeInDataStream(QDataStream& stream) override
		{
			toneMappingState.writeInDataStream(stream);
			cosmoCamState.writeInDataStream(stream);
			planetCamState.writeInDataStream(stream);
			stream << renderLabels;
			stream << renderOrbits;
			stream << compass;
			compassState.writeInDataStream(stream);
			stream << stereoMultiplier;
			universeState.writeInDataStream(stream);
		};

		ToneMappingModel::State toneMappingState;
		Camera::State cosmoCamState;
		OrbitalSystemCamera::State planetCamState;
		float renderLabels;
		float renderOrbits;
		bool compass = false;
		CalibrationCompass::State compassState;
		double stereoMultiplier = 1.0;
		Universe::State universeState;
	};

	MainWin();

	// CUBE

	/**
	 * @getter{renderOrbits}
	 */
	float renderOrbits() const
	{
		return CelestialBodyRenderer::renderOrbits();
	};
	/**
	 * @setter{renderOrbits, renderOrbits}
	 */
	void setRenderOrbits(float render)
	{
		CelestialBodyRenderer::renderOrbits() = render;
	};
	/**
	 * @getter{renderLabels}
	 */
	float renderLabels() const
	{
		return CelestialBodyRenderer::renderLabels();
	};
	/**
	 * @setter{renderLabels, renderLabels}
	 */
	void setRenderLabels(float render)
	{
		CelestialBodyRenderer::renderLabels() = render;
	};
	/**
	 * @getter{darkmatterEnabled}
	 */
	bool darkmatterEnabled() const { return Method::isDarkMatterEnabled(); };
	/**
	 * @setter{darkmatterEnabled, darkmatterEnabled}
	 */
	void setDarkmatterEnabled(bool enabled)
	{
		Method::setDarkMatterEnabled(enabled);
	};
	/**
	 * @getter{gridEnabled}
	 */
	bool gridEnabled() const { return showGrid; };
	/**
	 * @setter{gridEnabled, gridEnabled}
	 */
	void setGridEnabled(bool enabled) { showGrid = enabled; };

  public slots:
	/**
	 * @brief Toggles the @e gridEnabled property.
	 */
	void toggleGrid() { setGridEnabled(!gridEnabled()); };

	QString getVoiceoverPath()
	{
		if(isServer())
		{
			return QSettings().value("data/rootdir").toString() + "/voiceover/"
			       + (scenes->voiceOverIsEnglish() ? "EN" : "JP") + ".wav";
		}
		else
		{
			return "";
		}
	};

  protected:
	virtual void actionEvent(BaseInputManager::Action const& a,
	                         bool pressed) override;
	virtual bool event(QEvent* e) override;
	virtual void mousePressEvent(QMouseEvent* e) override;
	virtual void mouseReleaseEvent(QMouseEvent* e) override;
	virtual void mouseMoveEvent(QMouseEvent* e) override;
	virtual void wheelEvent(QWheelEvent* e) override;
	virtual void vrEvent(VRHandler::Event const& e) override;
	virtual void gamepadEvent(GamepadHandler::Event const& e) override;

	virtual void setupPythonAPI() override;
	virtual void initLibraries() override;

	// declare drawn resources
	virtual void initScene() override;

	// update physics/controls/meshes, etc...
	// prepare for rendering
	virtual void updateScene(BasicCamera& camera,
	                         QString const& pathId) override;

	// render user scene on camera
	// (no controllers or hands)
	virtual void renderScene(BasicCamera const& camera,
	                         QString const& pathId) override;
	virtual void renderGui(QSize const& /*targetSize*/) override;

	virtual void applyPostProcShaderParams(
	    QString const& id, GLShaderProgram const& shader,
	    GLFramebufferObject const& currentTarget) const override;

	virtual std::vector<
	    std::pair<GLTexture const*, GLComputeShader::DataAccessMode>>
	    getPostProcessingUniformTextures(
	        QString const& id, GLShaderProgram const& shader,
	        GLFramebufferObject const& currentTarget) const override;

	virtual std::unique_ptr<AbstractState> constructNewState() const override
	{
		return std::make_unique<MainWin::State>();
	};
	virtual void readState(AbstractState const& s) override
	{
		auto const& state = dynamic_cast<State const&>(s);
		toneMappingModel->readState(state.toneMappingState);

		auto& cam(renderer.getCamera<Camera&>("cosmo"));
		cam.readState(state.cosmoCamState);
		if(universe->isPlanetarySystemLoaded())
		{
			try
			{
				auto& cam2(renderer.getCamera<OrbitalSystemCamera&>("planet"));
				cam2.readState(state.planetCamState);
			}
			catch(...)
			{
			}
		}
		CelestialBodyRenderer::renderLabels() = state.renderLabels;
		CelestialBodyRenderer::renderOrbits() = state.renderOrbits;
		renderer.setCalibrationCompass(state.compass);
		CalibrationCompass::readState(state.compassState);
		vrHandler->setStereoMultiplier(state.stereoMultiplier);
		universe->readState(state.universeState);
	};
	virtual void writeState(AbstractState& s) const override
	{
		auto& state = dynamic_cast<State&>(s);
		toneMappingModel->writeState(state.toneMappingState);

		auto const& cam(renderer.getCamera<Camera const&>("cosmo"));
		cam.writeState(state.cosmoCamState);
		if(universe->isPlanetarySystemLoaded())
		{
			auto const& cam2(
			    renderer.getCamera<OrbitalSystemCamera const&>("planet"));
			cam2.writeState(state.planetCamState);
		}
		state.renderLabels = CelestialBodyRenderer::renderLabels();
		state.renderOrbits = CelestialBodyRenderer::renderOrbits();
		state.compass      = renderer.getCalibrationCompass();
		CalibrationCompass::writeState(state.compassState);
		state.stereoMultiplier = vrHandler->getStereoMultiplier();
		universe->writeState(state.universeState);
	};

  private:
	void printPositionInDataSpace(Side controller = Side::NONE) const;
	static std::vector<float> generateVertices(unsigned int number,
	                                           unsigned int seed);

	bool loaded = false;
	std::unique_ptr<Universe> universe;
	std::unique_ptr<Grid> grid;
	bool showGrid = QSettings().value("misc/showgrid").toBool();

	bool moveView = false;
	QPoint cursorPosBackup;
	QElapsedTimer cursorTimer;
	std::unique_ptr<MovementControls> movementControls;

	/* TEXT */
	bool showHelperBillboard
	    = QSettings().value("misc/halfcavehelper").toBool();
	double uiLabelsSizeMul
	    = QSettings().value("misc/uilabelssizemul").toDouble();
	std::unique_ptr<Billboard> helperBillboard;
	std::unique_ptr<Text3D> debugText;
	double timeSinceTextUpdate = DBL_MAX;
	bool showInfoText          = false;

	// TEMP
	const int textWidth  = 225;
	const int textHeight = 145;

	std::string lastTargetName = std::string("");

	// LENSING
	std::unique_ptr<GLTexture> lenseDistortionMap;
	QVector4D lenseScreenCoord;
	float lenseDist = 0.f;

	// UI
	std::unique_ptr<Visibilities> visibilities;
	std::unique_ptr<PlanetarySystemSelector> planetSysSelect;
	std::unique_ptr<UniverseElementSelector> univElemSelect;
	std::unique_ptr<TimeController> timeController;
	std::unique_ptr<AnimationTimeSelector> animTimeSelect;
	std::unique_ptr<ToneMappingController> tmController;
	std::unique_ptr<SceneSelector> scenes;
	std::unique_ptr<PresenterHelp> presenterHelp;

	// scenes
	std::unique_ptr<Animator> animator;
	QSoundEffect ambiance;
	QSoundEffect inSound;
	QSoundEffect thrustSound;
	QSoundEffect outSound;
	// idle
	QElapsedTimer idleTimer;
	void stopIdle()
	{
		idleTimer.restart();
		if(animator->getIdleMode())
		{
			animator->setIdleMode(false);
			timeSinceTextUpdate = 0.0;
			debugText->setText("");
		}
	}
};

#endif // MAINWIN_H
