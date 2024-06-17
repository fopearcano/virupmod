#ifndef MAINWIN_H
#define MAINWIN_H

#include <QCalendarWidget>

#include "AbstractMainWin.hpp"
#include "Billboard.hpp"
#include "CalibrationCompass.hpp"
#include "DemoDialog.hpp"
#include "Model.hpp"
#include "Primitives.hpp"
#include "ShaderProgram.hpp"
#include "Text3D.hpp"
#include "Volume.hpp"
#include "Widget3D.hpp"
#include "movingcube/MovingCube.hpp"

class MainWin : public AbstractMainWin
{
	Q_OBJECT
  public:
	class State : public AbstractState
	{
	  public:
		State()                   = default;
		State(State const& other) = default;
		State(State&& other)      = default;
		virtual void readFromDataStream(QDataStream& stream) override
		{
			stream >> exposure;
			stream >> dynamicrange;
			stream >> yaw;
			stream >> pitch;
			stream >> compass;
			compassState.readFromDataStream(stream);
		};
		virtual void writeInDataStream(QDataStream& stream) override
		{
			stream << exposure;
			stream << dynamicrange;
			stream << yaw;
			stream << pitch;
			stream << compass;
			compassState.writeInDataStream(stream);
		};

		float exposure     = 0.f;
		float dynamicrange = 0.f;
		float yaw          = 0.f;
		float pitch        = 0.f;
		bool compass       = false;
		CalibrationCompass::State compassState;
	};

	MainWin() = default;

  protected:
	virtual void actionEvent(BaseInputManager::Action const& a,
	                         bool pressed) override;
	virtual bool event(QEvent* e) override;
	virtual void mousePressEvent(QMouseEvent* e) override;
	virtual void mouseReleaseEvent(QMouseEvent* e) override;
	virtual void mouseMoveEvent(QMouseEvent* e) override;
	virtual void gamepadEvent(GamepadHandler::Event const& e) override;

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

	virtual void renderGui(QSize const& targetSize, AdvancedPainter& painter) override;

	virtual void applyPostProcShaderParams(
	    QString const& id, GLShaderProgram const& shader,
	    GLFramebufferObject const& currentTarget) const override;

	virtual std::unique_ptr<AbstractState> constructNewState() const override
	{
		return std::make_unique<MainWin::State>();
	};
	virtual void readState(AbstractState const& s) override
	{
		auto const& state              = dynamic_cast<State const&>(s);
		toneMappingModel->exposure     = state.exposure;
		toneMappingModel->dynamicrange = state.dynamicrange;
		yaw                            = state.yaw;
		pitch                          = state.pitch;
		renderer.setCalibrationCompass(state.compass);
		CalibrationCompass::readState(state.compassState);
	};
	virtual void writeState(AbstractState& s) const override
	{
		auto& state        = dynamic_cast<State&>(s);
		state.exposure     = toneMappingModel->exposure;
		state.dynamicrange = toneMappingModel->dynamicrange;
		state.yaw          = yaw;
		state.pitch        = pitch;
		state.compass      = renderer.getCalibrationCompass();
		CalibrationCompass::writeState(state.compassState);
	};

  private:
	ShaderProgram sbShader;
	std::unique_ptr<GLMesh> skybox;
	std::unique_ptr<GLTexture> sbTexture;

	std::unique_ptr<GLMesh> mesh;
	ShaderProgram shaderProgram;

	std::unique_ptr<GLMesh> pointsMesh;
	ShaderProgram pointsShader;

	std::unique_ptr<MovingCube> movingCube;

	std::unique_ptr<GLMesh> sphere;
	ShaderProgram sphereShader;

	std::unique_ptr<GLMesh> playarea;
	ShaderProgram playareaShader;

	std::unique_ptr<Model> model;
	std::unique_ptr<Light> light;
	QMatrix4x4 modelModel;

	std::unique_ptr<Billboard> bill;
	std::unique_ptr<Text3D> text;
	QCalendarWidget calendar;
	std::unique_ptr<Widget3D> widget3d;
	std::unique_ptr<DemoDialog> dialog;

	std::unique_ptr<Volume> volume;

	QImage image;

	/* TEST ANISOTROPIC FILTERING
	std::unique_ptr<GLShaderProgram> largeGridShader;
	std::unique_ptr<GLTexture> largeGridTex;
	std::unique_ptr<GLMesh> largeGridMesh;*/

	float barrelPower = 1.01f;

	bool moveView = false;
	QPoint cursorPosBackup;
	float yaw        = 0.f;
	float pitch      = 0.f;
	QVector3D campos = QVector3D(1, 1, 1);

	QElapsedTimer timer;
};

#endif // MAINWIN_H
