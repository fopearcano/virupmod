/*
    Copyright (C) 2020 Florian Cabot <florian.cabot@hotmail.fr>

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

#include "universe/PlanetarySystems.hpp"

PlanetarySystems::PlanetarySystems()
    : shader("default")
{
	QString rootdir(QSettings().value("data/rootdir").toString());
	auto planetsystemdir
	    = rootdir + QSettings().value("simulation/planetsystemdir").toString();

	QStringList files;
	QDirIterator it(planetsystemdir, QStringList() << "*.json", QDir::Files,
	                QDirIterator::Subdirectories);
	while(it.hasNext())
	{
		files << it.next();
	}

	std::vector<float> vertices;
	for(auto const& file : files)
	{
		QFile jsonFile(file);
		if(jsonFile.exists())
		{
			jsonFile.open(QIODevice::ReadOnly);
			QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonFile.readAll());
			QString name(QFileInfo(jsonFile).dir().dirName());

			auto orbitalSystem
			    = new OrbitalSystem(name.toStdString(), jsonDoc.object());
			if(!orbitalSystem->isValid())
			{
				qWarning() << QString(orbitalSystem->getName().c_str())
				           << " is invalid... ";
				continue;
			}
			double dist(orbitalSystem->getDistanceToEarth());
			double ra(orbitalSystem->getRightAscension());
			double dec(orbitalSystem->getDeclination());
			Vector3 position(dist * cos(ra) * cos(dec),
			                 dist * sin(ra) * cos(dec), dist * sin(dec));
			vertices.push_back(position[0]);
			vertices.push_back(position[1]);
			vertices.push_back(position[2]);
			positions.push_back(position);
			systems.push_back(orbitalSystem);
			directories.push_back(QFileInfo(jsonFile).absoluteDir().path());
			ids[orbitalSystem->getName().c_str()] = systems.size() - 1;

			bbox.minx = std::min(bbox.minx, static_cast<float>(position[0]));
			bbox.miny = std::min(bbox.miny, static_cast<float>(position[1]));
			bbox.minz = std::min(bbox.minz, static_cast<float>(position[2]));

			bbox.maxx = std::max(bbox.maxx, static_cast<float>(position[0]));
			bbox.maxy = std::max(bbox.maxy, static_cast<float>(position[1]));
			bbox.maxz = std::max(bbox.maxz, static_cast<float>(position[2]));
		}
		bbox.mid = QVector3D((bbox.maxx + bbox.minx) * 0.5f,
		                     (bbox.maxy + bbox.miny) * 0.5f,
		                     (bbox.maxz + bbox.minz) * 0.5f);
		bbox.diameter
		    = sqrt(pow(bbox.maxx - bbox.minx, 2) + pow(bbox.maxy - bbox.miny, 2)
		           + pow(bbox.maxz - bbox.minz, 2));
	}
	QString solarsystemjson(
	    rootdir + QSettings().value("simulation/solarsystemdir").toString()
	    + "/definition.json");
	QFile jsonFile(solarsystemjson);
	if(jsonFile.exists())
	{
		jsonFile.open(QIODevice::ReadOnly);
		QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonFile.readAll());
		QString name(QFileInfo(jsonFile).dir().dirName());

		QString dir(QFileInfo(jsonFile).absoluteDir().path());
		PlanetRenderer::currentSystemDir = dir;
		CSVOrbit::currentSystemDir       = dir;

		auto orbitalSystem
		    = new OrbitalSystem(name.toStdString(), jsonDoc.object());
		if(!orbitalSystem->isValid())
		{
			qWarning() << QString(orbitalSystem->getName().c_str())
			           << " is invalid... ";
		}
		else
		{
			vertices.push_back(0.0);
			vertices.push_back(0.0);
			vertices.push_back(0.0);
			positions.emplace_back();
			systems.push_back(orbitalSystem);
			directories.push_back(dir);
			ids[orbitalSystem->getName().c_str()] = systems.size() - 1;
			qDebug() << dir;
		}
	}
	mesh.setVertexShaderMapping(shader, {{"position", 3}});
	mesh.setVertices(vertices);

	unit = 1.0e-3;

	shader.setUniform("color", QVector3D(1000000.f, 0.f, 0.f));

	PlanetRenderer::currentSystemDir = directories[closestId];
	CSVOrbit::currentSystemDir       = directories[closestId];
}

QStringList PlanetarySystems::getSystemsNames() const
{
	QStringList result;
	for(auto const& pair : ids)
	{
		result << pair.first;
	}
	return result;
}

Vector3 PlanetarySystems::getAbsolutePosition(QString const& systemName) const
{
	auto relativePos(positions[ids.at(systemName)]);
	return Utils::fromQt(getRelToAbsTransform() * Utils::toQt(relativePos));
}

void PlanetarySystems::update(Camera const& camera)
{
	getModelAndCampos(camera, model, campos);

	QVector3D pos = useVRCamposForClosest ? campos
	                                      : getRelToAbsTransform().inverted()
	                                            * Utils::toQt(camera.position);

	double dist(DBL_MAX);
	unsigned int oldClosestId(closestId);
	for(unsigned int i(0); i < positions.size(); ++i)
	{
		double d((positions[i] - Utils::fromQt(pos)).length());
		if(d < dist)
		{
			dist      = d;
			closestId = i;
		}
	}
	double neighborDist(DBL_MAX);
	for(unsigned int i(0); i < positions.size(); ++i)
	{
		double d((positions[i] - positions[closestId]).length());
		if(d < neighborDist && i != closestId)
		{
			neighborDist = d;
		}
	}
	// put max neighborDist
	neighborDist                     = neighborDist > 2 ? 2 : neighborDist;
	PlanetRenderer::currentSystemDir = directories[closestId];
	CSVOrbit::currentSystemDir       = directories[closestId];

	if(camera.scale * neighborDist > 2000
	   && (!doRender || oldClosestId != closestId))
	{
		doRender = true;
	}
	else if(camera.scale * neighborDist <= 2000)
	{
		doRender = false;
	}
}

void PlanetarySystems::render(Camera const& /*camera*/,
                              ToneMappingModel const& tmm)
{
	GLHandler::beginTransparent();
	shader.setUniform("alpha", getVisibility());
	shader.setUniform("exposure", tmm.exposure);
	shader.setUniform("dynamicrange", tmm.dynamicrange);
	GLHandler::setUpRender(shader, model);
	mesh.render();
	GLHandler::endTransparent();
}

// NOLINTNEXTLINE(hicpp-use-equals-default,modernize-use-equals-default)
PlanetarySystems::~PlanetarySystems()
{
	for(auto sys : systems)
	{
		delete sys;
	}
}
