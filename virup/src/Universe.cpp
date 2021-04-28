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

#include "Universe.hpp"

Universe::Universe(OrbitalSystemCamera& camPlanet)
    : camPlanet(camPlanet)
{
	cosmologicalSim = new CosmologicalSimulation(
	    QSettings().value("data/gazfile").toString().toStdString(),
	    QSettings().value("data/starsfile").toString().toStdString(),
	    QSettings().value("data/loaddarkmatter").toBool()
	        ? QSettings().value("data/darkmatterfile").toString().toStdString()
	        : "");
	cosmologicalSim->referenceFrame = UniverseElement::ReferenceFrame::GALACTIC;
	cosmologicalSim->unit           = 1.0;
	cosmologicalSim->solarsystemPosition  = Vector3(-8.29995608, 0.0, 0.027);
	cosmologicalSim->brightnessMultiplier = 1e7;
	updateBoundingBox(cosmologicalSim->getBoundingBox());

	hyg       = new CSVObjects(QSettings().value("data/hyg").toString(),
                         QSettings().value("data/hygcon").toString());
	hyg->unit = 0.001;
	updateBoundingBox(hyg->getBoundingBox());

	sdss = new CSVObjects(QSettings().value("data/sdss").toString(), true);
	sdss->unit                 = 1000.0;
	sdss->brightnessMultiplier = 1e9;
	updateBoundingBox(sdss->getBoundingBox());

	planetSystems = new PlanetarySystems;
	updateBoundingBox(planetSystems->getBoundingBox());

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
	loadClosestSystem();
}

QString Universe::getPlanetTarget() const
{
	return camPlanet.target->getName().c_str();
}

void Universe::setPlanetTarget(QString const& name)
{
	auto ptrs = orbitalSystem->getAllCelestialBodiesPointers();
	for(auto ptr : ptrs)
	{
		if(QString(ptr->getName().c_str()) == name)
		{
			camPlanet.target = ptr;
		}
	}
}

QString Universe::getClosestCommonAncestorName(
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

Vector3 Universe::getCelestialBodyPosition(QString const& bodyName,
                                           QString const& referenceBodyName,
                                           QDateTime const& dt,
                                           UniversalTime const& currentUt) const
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
	return Orbitable::getRelativePositionAtUt(orbRef, orb, currentUt);
}

Vector3 Universe::interpolateCoordinates(QString const& celestialBodyName0,
                                         QString const& celestialBodyName1,
                                         float t,
                                         UniversalTime const& currentUt) const
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

	return (Orbitable::getRelativePositionAtUt(ancestor, orb0, currentUt)
	        * (1 - t))
	       + (Orbitable::getRelativePositionAtUt(ancestor, orb1, currentUt)
	          * t);
}

void Universe::updateCosmo(Camera const& cam)
{
	cosmologicalSim->update(cam);
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
		double rescale(pos.length() <= 8000.0 ? 1.0 : 8000.0 / pos.length());
		QMatrix4x4 model;
		model.translate(Utils::toQt(pos * rescale));
		model.scale(rescale * camRelPos.length() * cam.scale / 3.0);
		model.rotate(yaw * 180.f / M_PI + 90.f, 0.0, 0.0, 1.0);
		model.rotate(pitch * 180.f / M_PI + 90.f, 1.0, 0.0, 0.0);
		cosmoLabel.second->updateModel(model);
	}
}

void Universe::updatePlanetarySystem(Camera const& cam,
                                     UniversalTime const& currentUt)
{
	if(!planetSystems->renderSystem())
	{
		return;
	}
	const double mtokpc = 3.24078e-20;
	if(lastData != planetSystems->getClosestSystemPosition())
	{
		planetarySystemName = "";
		loadClosestSystem();
	}

	lastData   = planetSystems->getClosestSystemPosition();
	sysInWorld = cam.dataToWorldPosition(lastData);

	CelestialBodyRenderer::overridenScale = mtokpc * cam.scale;

	if((camPlanet.target == orbitalSystem->getMainCelestialBody()
	    && CelestialBodyRenderer::overridenScale < 1e-12)
	   || forceUpdateFromCosmo)
	{
		camPlanet.relativePosition = -1 * sysInWorld / (mtokpc * cam.scale);
		forceUpdateFromCosmo       = false;
	}
	sysInWorld = cam.dataToWorldPosition(lastData);
	systemRenderer->updateMesh(currentUt, camPlanet);
}

void Universe::renderCosmo(Camera const& cam,
                           ToneMappingModel const& toneMappingModel)
{
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
			   && planetSystems->getClosestSystem()->getName()
			          == "Solar System")
			{
				continue;
			}
			cosmoLabel.second->render(toneMappingModel.exposure,
			                          toneMappingModel.dynamicrange);
		}
	}
	GLHandler::glf().glDisable(GL_CLIP_DISTANCE0);
	GLHandler::glf().glDisable(GL_DEPTH_CLAMP);
}

void Universe::renderPlanetarySystem()
{
	systemRenderer->render(camPlanet);
}

void Universe::renderPlanetarySystemTransparent()
{
	systemRenderer->renderTransparent(camPlanet);
}

void Universe::updateBoundingBox(BBox const& elementBoundingbox)
{
	boundingBox.minx = fmin(boundingBox.minx, elementBoundingbox.minx);
	boundingBox.maxx = fmin(boundingBox.maxx, elementBoundingbox.maxx);
	boundingBox.miny = fmin(boundingBox.miny, elementBoundingbox.miny);
	boundingBox.maxy = fmin(boundingBox.maxy, elementBoundingbox.maxy);
	boundingBox.minz = fmin(boundingBox.minz, elementBoundingbox.minz);
	boundingBox.maxz = fmin(boundingBox.maxz, elementBoundingbox.maxz);

	boundingBox.diameter = sqrtf((boundingBox.maxx - boundingBox.minx)
	                                 * (boundingBox.maxx - boundingBox.minx)
	                             + (boundingBox.maxy - boundingBox.miny)
	                                   * (boundingBox.maxy - boundingBox.miny)
	                             + (boundingBox.maxz - boundingBox.minz)
	                                   * (boundingBox.maxz - boundingBox.minz));

	boundingBox.mid.setX((boundingBox.maxx + boundingBox.minx) / 2.0f);
	boundingBox.mid.setY((boundingBox.maxy + boundingBox.miny) / 2.0f);
	boundingBox.mid.setZ((boundingBox.maxz + boundingBox.minz) / 2.0f);
}

void Universe::loadClosestSystem()
{
	delete systemRenderer;

	orbitalSystem  = planetSystems->getClosestSystem();
	systemRenderer = new OrbitalSystemRenderer(orbitalSystem);

	/*debugText->setText(QString(orbitalSystem->getName().c_str()));
	lastTargetName = orbitalSystem->getMainCelestialBody()->getName();
	timeSinceTextUpdate = 0.f;*/

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

	camPlanet.target           = orbitalSystem->getMainCelestialBody();
	camPlanet.relativePosition = Vector3(
	    camPlanet.target->getCelestialBodyParameters().radius * 2.0, 0.0, 0.0);

	CelestialBodyRenderer::overridenScale = 1.0;
	forceUpdateFromCosmo                  = true;

	planetarySystemName = orbitalSystem->getName().c_str();
}

Universe::~Universe()
{
	delete systemRenderer;
	for(auto cosmoLabel : cosmoLabels)
	{
		delete cosmoLabel.second;
	}
	delete planetSystems;
	delete sdss;
	delete hyg;
	delete cosmologicalSim;
}
