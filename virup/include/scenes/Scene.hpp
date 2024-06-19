/*
    Copyright (C) 2021 Florian Cabot <florian.cabot@hotmail.fr>

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

#ifndef SCENE_HPP
#define SCENE_HPP

#include "SceneCameraData.hpp"
#include "SceneSpatialData.hpp"
#include "SceneTemporalData.hpp"
#include "SceneToneMappingData.hpp"
#include "SceneUI.hpp"

/*! \ingroup pywrap
 *
 * To use this class in a Python script :
 * \code{.py}
 * from PythonQt.virup import Scene
 * \endcode
 */
class Scene
{
  public:
	Scene()                   = default;
	Scene(Scene const& other) = default;
	Scene(Scene&& other)      = default;
	Scene(SceneSpatialData sd, SceneTemporalData td, SceneUI ui,
	      SceneCameraData cd = {}, SceneToneMappingData tm = {});
	Scene& operator=(Scene const& other) = default;

	SceneSpatialData const& getSpatialData() const { return sd; };
	void setSpatialData(SceneSpatialData sd) { this->sd = sd; };
	SceneTemporalData const& getTemporalData() const { return td; };
	void setTemporalData(SceneTemporalData td) { this->td = td; };
	SceneUI const& getUI() const { return ui; };
	void setUI(SceneUI ui) { this->ui = ui; };
	SceneCameraData const& getCameraData() const { return cd; };
	void setCameraData(SceneCameraData cd) { this->cd = cd; };
	SceneToneMappingData const& getToneMappingData() const { return tmd; };
	void setToneMappingData(SceneToneMappingData tmd) { this->tmd = tmd; };

	static Scene getCurrentState(Universe const& universe,
	                             ToneMappingModel const& tmm);
	void setAsUniverseState(Universe& universe, ToneMappingModel& tmm);

	static Scene interpolate(Scene const& s0, Scene const& s1, float t);

	QString getPythonRepresentation() const;

  private:
	SceneSpatialData sd;
	SceneTemporalData td;
	SceneUI ui;
	SceneCameraData cd;
	SceneToneMappingData tmd;
};

/* PYTHONQT */

Q_DECLARE_METATYPE(Scene)

class SceneWrapper : public PythonQtWrapper
{
	Q_OBJECT
  public:
	virtual const char* wrappedClassName() const override { return "Scene"; }
	virtual const char* wrappedClassPackage() const override { return "virup"; }

  public Q_SLOTS:
	Scene* new_Scene() { return new Scene; }
	Scene* new_Scene(Scene const& s) { return new Scene(s); }
	Scene* new_Scene(SceneSpatialData sd, SceneTemporalData td, SceneUI ui)
	{
		return new Scene(sd, td, ui);
	}
	Scene* new_Scene(SceneSpatialData const& sd, SceneTemporalData const& td,
	                 SceneUI const& ui, SceneCameraData const& cd)
	{
		return new Scene(sd, td, ui, cd);
	}
	Scene* new_Scene(SceneSpatialData const& sd, SceneTemporalData const& td,
	                 SceneUI const& ui, SceneCameraData const& cd,
	                 SceneToneMappingData const& tm)
	{
		return new Scene(sd, td, ui, cd, tm);
	}

	void delete_Scene(Scene* s) { delete s; }

	// access methods

	SceneSpatialData getSpatialData(Scene* s) const
	{
		return s->getSpatialData();
	};
	void setSpatialData(Scene* s, SceneSpatialData sd)
	{
		s->setSpatialData(sd);
	};
	SceneTemporalData getTemporalData(Scene* s) const
	{
		return s->getTemporalData();
	};
	void setTemporalData(Scene* s, SceneTemporalData td)
	{
		s->setTemporalData(td);
	};
	SceneUI getUI(Scene* s) const { return s->getUI(); };
	void setUI(Scene* s, SceneUI ui) { s->setUI(ui); };
	SceneCameraData getCameraData(Scene* s) const
	{
		return s->getCameraData();
	};
	void setCameraData(Scene* s, SceneCameraData cd) { s->setCameraData(cd); };
	SceneToneMappingData getToneMappingData(Scene* s) const
	{
		return s->getToneMappingData();
	};
	void setToneMappingData(Scene* s, SceneToneMappingData tm)
	{
		s->setToneMappingData(tm);
	};

	Scene static_Scene_getCurrentState(Universe const& universe,
	                                   ToneMappingModel const& tmm)
	{
		return Scene::getCurrentState(universe, tmm);
	};
	void setAsUniverseState(Scene* s, Universe& universe,
	                        ToneMappingModel& tmm) const
	{
		s->setAsUniverseState(universe, tmm);
	};
	Scene static_Scene_interpolate(Scene const& s0, Scene const& s1, float t)
	{
		return Scene::interpolate(s0, s1, t);
	}
};

#endif // SCENE_HPP
