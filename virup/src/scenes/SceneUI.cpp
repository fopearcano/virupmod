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

#include "scenes/SceneUI.hpp"

SceneUI::SceneUI(std::map<QString, float> visibilities)
    : visibilities(std::move(visibilities))
{
}

SceneUI::SceneUI(QVariantMap const& visibilities)
    : visibilities(fromQt(visibilities))
{
}

float SceneUI::getVisibility(QString const& name) const
{
	if(visibilities.count(name) == 0)
	{
		return 0.f;
	}
	return visibilities.at(name);
}

void SceneUI::setVisibility(QString const& name, float vis)
{
	visibilities[name] = vis;
}

SceneUI SceneUI::getCurrentState(Universe const& universe)
{
	QStringList n = {"Constellations", "Orbits", "PlanetsLabels", "Debris"};
	n.append(universe.getUniverseElementsNames());

	std::map<QString, float> vis;
	for(auto const& name : n)
	{
		vis[name] = universe.getVisibility(name);
	}

	return {vis};
}

void SceneUI::setAsUniverseState(Universe& universe) const
{
	QStringList n = {"Constellations", "Orbits", "PlanetsLabels", "Debris"};
	n.append(universe.getUniverseElementsNames());

	for(auto const& name : n)
	{
		universe.setVisibility(name, getVisibility(name));
	}
}

SceneUI SceneUI::interpolate(SceneUI const& ui0, SceneUI const& ui1, float t)
{
	std::map<QString, float> vis;

	QStringList names;

	for(auto const& pair : ui0.visibilities)
	{
		if(!names.contains(pair.first))
		{
			names.append(pair.first);
		}
	}
	for(auto const& pair : ui1.visibilities)
	{
		if(!names.contains(pair.first))
		{
			names.append(pair.first);
		}
	}

	for(auto const& name : names)
	{
		vis[name]
		    = ui0.getVisibility(name) * (1.f - t) + ui1.getVisibility(name) * t;
	}

	return {vis};
}

QString SceneUI::getPythonRepresentation() const
{
	QString result("SceneUI({");

	for(auto const& pair : visibilities)
	{
		if(pair.second == 0.f)
		{
			continue;
		}
		result += '"' + pair.first + "\":" + QString::number(pair.second) + ',';
	}

	result += "})";
	return result;
}

std::map<QString, float> SceneUI::fromQt(QVariantMap const& map)
{
	std::map<QString, float> result;
	for(auto const& key : map.keys())
	{
		result[key] = map.value(key).toFloat();
	}
	return result;
}
