#ifndef MAINWIN_H
#define MAINWIN_H


#include "AbstractMainWin.hpp"

class MainWin : public AbstractMainWin
{
	Q_OBJECT
  public:
	MainWin() = default;

  protected:
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
};

#endif // MAINWIN_H
