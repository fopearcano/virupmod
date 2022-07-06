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

#include "scenes/Scene.hpp"

Scene::Scene(SceneSpatialData sd, SceneTemporalData td, SceneUI ui,
             SceneCameraData cd)
    : sd(std::move(sd))
    , td(std::move(td))
    , ui(std::move(ui))
    , cd(std::move(cd))
{
}

Scene Scene::getCurrentState(Universe const& universe)
{
	return {SceneSpatialData::getCurrentState(universe),
	        SceneTemporalData::getCurrentState(universe),
	        SceneUI::getCurrentState(universe),
	        SceneCameraData::getCurrentState(universe)};
}

void Scene::setAsUniverseState(Universe& universe)
{
	sd.setAsUniverseState(universe);
	td.setAsUniverseState(universe);
	ui.setAsUniverseState(universe);
	cd.setAsUniverseState(universe);
}

Scene Scene::interpolate(Scene const& s0, Scene const& s1, float t)
{
	return {SceneSpatialData::interpolate(s0.sd, s1.sd, t),
	        SceneTemporalData::interpolate(s0.td, s1.td, t),
	        SceneUI::interpolate(s0.ui, s1.ui, t),
	        SceneCameraData::interpolate(s0.cd, s1.cd, t)};
}

QString Scene::getPythonRepresentation() const
{
	QString result("Scene(");
	result += sd.getPythonRepresentation() + ",\n    ";
	result += td.getPythonRepresentation() + ", ";
	result += ui.getPythonRepresentation() + ",\n    ";
	result += cd.getPythonRepresentation();
	result += ')';

	return result;
}
