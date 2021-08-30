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

unsigned int Universe::State::elementsSize = 0;

Universe::Universe(OrbitalSystemCamera& camPlanet)
    : camPlanet(camPlanet)
{
	QJsonDocument jsondoc(QJsonDocument::fromJson(
	    QSettings().value("data/json").toString().toLatin1()));
	QJsonObject dataJsonRepresentation = jsondoc.object();
	if(dataJsonRepresentation.keys().indexOf("entries") == -1)
	{
		dataJsonRepresentation["entries"] = QJsonArray();
	}
	for(auto entry : dataJsonRepresentation["entries"].toArray())
	{
		auto entryObj(entry.toObject());
		UniverseElement* newElem = nullptr;
		if(entryObj["type"] == "cosmolabels")
		{
			newElem = new CosmologicalLabels(entryObj);
		}
		else if(entryObj["type"] == "csvstars")
		{
			auto csv = new CSVObjects(entryObj, false);
			newElem  = csv;
			csvObjs.append(csv);
		}
		else if(entryObj["type"] == "csvgalaxies")
		{
			auto csv = new CSVObjects(entryObj, true);
			newElem  = csv;
			csvObjs.append(csv);
		}
		else if(entryObj["type"] == "cosmosim")
		{
			auto cs = new CosmologicalSimulation(entryObj);
			newElem = cs;
			cosmoSims.append(cs);
		}
		else if(entryObj["type"] == "texsphere")
		{
			newElem = new TexturedSphere(entryObj);
		}
		else if(entryObj["type"] == "credits")
		{
			newElem = new Credits(entryObj);
		}
		else
		{
			qWarning()
			    << "Entry JSON Object is not valid. (See following warning)";
			qWarning() << entryObj;
			continue;
		}
		newElem->unit          = entryObj["unit"].toDouble(1.0);
		QString referenceFrame = entryObj["referenceframe"].toString();
		if(referenceFrame == "equatorial")
		{
			newElem->referenceFrame
			    = UniverseElement::ReferenceFrame::EQUATORIAL;
		}
		else if(referenceFrame == "galactic")
		{
			newElem->referenceFrame = UniverseElement::ReferenceFrame::GALACTIC;
		}
		else if(referenceFrame == "ecliptic")
		{
			newElem->referenceFrame = UniverseElement::ReferenceFrame::ECLIPTIC;
		}
		newElem->solarsystemPosition
		    = Vector3(entryObj["solarsyslocalpos"].toObject());
		newElem->brightnessMultiplier = entryObj["brightnessmul"].toDouble(1.0);
		updateBoundingBox(newElem->getBoundingBox());
		elements[entryObj["name"].toString()] = newElem;
	}

	planetSystems = new PlanetarySystems;
	updateBoundingBox(planetSystems->getBoundingBox());

	loadClosestSystem();

	// preload octrees data to fill VRAM giving priority to top levels
	uint64_t max(OctreeLOD::getMemLimit());

	uint64_t wholeData(0);
	for(auto cosmoSim : cosmoSims)
	{
		wholeData += cosmoSim->getOctreesTotalDataSize();
	}
	wholeData *= sizeof(float);

	QProgressDialog progress(QObject::tr("Preloading trees data..."), QString(),
	                         0, max < wholeData ? max : wholeData);
	progress.setMinimumDuration(0);
	progress.setValue(0);

	bool cont(true);
	for(unsigned int lvlToLoad(0); cont && lvlToLoad < 10; ++lvlToLoad)
	{
		for(int i(0); cont && i < cosmoSims.size(); ++i)
		{
			cont = cosmoSims[i]->preloadOctreesLevel(lvlToLoad, &progress);
		}
	}

	State::elementsSize = elements.size();

	PythonQtHandler::addObject("Universe", this);
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

void Universe::setVisibility(QString const& name, double visibility)
{
	if(name == "Exoplanets")
	{
		planetSystems->visibility = visibility;
		return;
	}
	if(name == "Constellations")
	{
		for(auto csv : csvObjs)
		{
			csv->constellationsAlpha  = visibility;
			csv->constellationsLabels = visibility;
		}
		return;
	}

	elements[name]->visibility = visibility;
}

void Universe::updateCosmo(Camera const& cam)
{
	OctreeLOD::updateTanAngleLimit(cam);
	for(auto pair : elements)
	{
		if(pair.second->visibility < 0.001)
		{
			continue;
		}
		pair.second->update(cam);
	}
	planetSystems->update(cam);
	planetSystems->useVRCamposForClosest
	    = PythonQtHandler::getVariable("id").toInt() == -1;
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
	planetSystems->getClosestSystem()->update(currentUt);
	systemRenderer->updateMesh(currentUt, camPlanet);
}

void Universe::renderCosmo(Camera const& cam,
                           ToneMappingModel const& toneMappingModel)
{
	GLHandler::glf().glDepthFunc(GL_LEQUAL);
	GLHandler::glf().glEnable(GL_DEPTH_CLAMP);
	GLHandler::glf().glEnable(GL_CLIP_DISTANCE0);
	for(auto pair : elements)
	{
		// only used by CosmologicalLabels for now
		// pair.second->visibility = CelestialBodyRenderer::renderLabels;
		if(pair.second->visibility < 0.001)
		{
			continue;
		}
		pair.second->render(cam, toneMappingModel);
	}

	planetSystems->render(cam, toneMappingModel);

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
	delete planetSystems;
	for(auto const& pair : elements)
	{
		delete pair.second;
	}
}
