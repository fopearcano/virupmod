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

#include "universe/Universe.hpp"

unsigned int Universe::State::elementsSize = 0;

Universe::Universe(Camera& camCosmo, OrbitalSystemCamera& camPlanet)
    : camCosmo(camCosmo)
    , camPlanet(camPlanet)
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
		newElem->setProperRotationFromCustomZAxis(
		    Utils::toQt(Vector3(entryObj["customzaxis"].toObject())));
		newElem->brightnessMultiplier = entryObj["brightnessmul"].toDouble(1.0);
		updateBoundingBox(newElem->getBoundingBox());
		elements[entryObj["name"].toString()] = newElem;
		elementsRev[newElem]                  = entryObj["name"].toString();
	}

	planetSystems = new PlanetarySystems;
	updateBoundingBox(planetSystems->getBoundingBox());
	elements["Exoplanets"] = planetSystems;

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

double Universe::getScale() const
{
	return camCosmo.scale * mtokpc;
}

void Universe::setScale(double scale)
{
	camCosmo.scale                        = scale / mtokpc;
	CelestialBodyRenderer::overridenScale = scale;
}

Vector3 Universe::getCosmoPosition() const
{
	return camCosmo.position;
}

void Universe::setCosmoPosition(Vector3 cosmoPosition)
{
	Vector3 diff(cosmoPosition - camCosmo.position);
	camCosmo.position = cosmoPosition;
	camPlanet.relativePosition += diff / mtokpc;
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

Vector3 Universe::getPlanetPosition() const
{
	return camPlanet.relativePosition;
}

void Universe::setPlanetPosition(Vector3 planetPosition)
{
	Vector3 diff(planetPosition - camPlanet.relativePosition);
	camPlanet.relativePosition = planetPosition;
	camCosmo.position += diff * mtokpc;
}

QDateTime Universe::getSimulationTime() const
{
	return SimulationTime::utToDateTime(clock.getCurrentUt());
}

void Universe::setSimulationTime(QDateTime const& simulationTime)
{
	clock.setCurrentUt(SimulationTime::dateTimeToUT(simulationTime, false));
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
                                           QDateTime const& dt) const
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
	return Orbitable::getRelativePositionAtUt(orbRef, orb,
	                                          clock.getCurrentUt());
}

Vector3 Universe::interpolateCoordinates(QString const& celestialBodyName0,
                                         QString const& celestialBodyName1,
                                         float t) const
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

	return (Orbitable::getRelativePositionAtUt(ancestor, orb0,
	                                           clock.getCurrentUt())
	        * (1 - t))
	       + (Orbitable::getRelativePositionAtUt(ancestor, orb1,
	                                             clock.getCurrentUt())
	          * t);
}

Vector3 Universe::getCameraCurrentRelPosToBody(QString const& bodyName) const
{
	Orbitable const* orb(nullptr);

	auto ptrs = orbitalSystem->getAllCelestialBodiesPointers();
	for(auto ptr : ptrs)
	{
		if(QString(ptr->getName().c_str()) == bodyName)
		{
			orb = ptr;
		}
	}
	if(orb == nullptr)
	{
		return {};
	}
	return camPlanet.getRelativePositionTo(orb, lastCurrentUt);
}

double Universe::getVisibility(QString const& name) const
{
	if(name == "Constellations")
	{
		for(auto csv : csvObjs)
		{
			return csv->constellationsAlpha;
		}
	}
	if(name == "Orbits")
	{
		return CelestialBodyRenderer::renderOrbits;
	}
	if(name == "PlanetsLabels")
	{
		return CelestialBodyRenderer::renderLabels;
	}

	if(elements.count(name) == 0)
	{
		qWarning() << name + " is not a valid UniverseElement";
		return 0.0;
	}

	return elements.at(name)->getVisibility();
}

void Universe::setVisibility(QString const& name, double visibility)
{
	if(name == "Constellations")
	{
		bool sendSig(false);
		for(auto csv : csvObjs)
		{
			if(visibility != csv->constellationsAlpha
			   || visibility != csv->constellationsLabels)
			{
				csv->constellationsAlpha  = visibility;
				csv->constellationsLabels = visibility;
				sendSig                   = true;
			}
			if(sendSig)
			{
				emit nonElementVisibilityChanged(name, visibility);
			}
		}
		return;
	}
	if(name == "Orbits")
	{
		if(CelestialBodyRenderer::renderOrbits != visibility)
		{
			CelestialBodyRenderer::renderOrbits = visibility;
			emit nonElementVisibilityChanged(name, visibility);
		}
		return;
	}
	if(name == "PlanetsLabels")
	{
		if(CelestialBodyRenderer::renderLabels != visibility)
		{
			CelestialBodyRenderer::renderLabels = visibility;
			emit nonElementVisibilityChanged(name, visibility);
		}
		return;
	}

	if(elements.count(name) == 0)
	{
		qWarning() << name + " is not a valid UniverseElement";
		return;
	}

	elements[name]->setVisibility(visibility);
}

Vector3 Universe::getSolarSystemPosition(QString const& name) const
{
	if(elements.count(name) == 0)
	{
		qWarning() << name + " is not a valid UniverseElement";
		return {};
	}

	return elements.at(name)->solarsystemPosition;
}

void Universe::setSolarSystemPosition(QString const& name, Vector3 const& pos)
{
	if(elements.count(name) == 0)
	{
		qWarning() << name + " is not a valid UniverseElement";
		return;
	}

	elements[name]->solarsystemPosition = pos;
}

void Universe::setLabelsOrbitsOnly(QStringList const& nameList)
{
	CelestialBodyRenderer::renderLabelsOrbitsOnly = nameList;
}

void Universe::dumpOctreesStates()
{
	for(auto cosmoSim : cosmoSims)
	{
		auto name(elementsRev.at(cosmoSim));
		cosmoSim->dumpOctreesStates(
		    QSettings().value("misc/octreestatesdir").toString(),
		    name.toLower().replace(' ', '_'));
	}
}

void Universe::updateCosmo()
{
	OctreeLOD::updateTanAngleLimit(camCosmo);
	for(auto pair : elements)
	{
		if(pair.second->getVisibility() < 0.001)
		{
			continue;
		}
		pair.second->update(camCosmo);
	}
	planetSystems->update(camCosmo);
	planetSystems->useVRCamposForClosest
	    = PythonQtHandler::getVariable("id").toInt() == -1;
}

void Universe::updateClock(bool videomode, float frameTiming)
{
	if(videomode)
	{
		clock.update(frameTiming);
	}
	else
	{
		clock.update();
	}
	camPlanet.updateUT(clock.getCurrentUt());
}

void Universe::updatePlanetarySystem()
{
	auto currentUt = clock.getCurrentUt();
	lastCurrentUt  = currentUt;
	if(!planetSystems->renderSystem())
	{
		return;
	}
	const double mtokpc = 3.24078e-20;
	if(lastData != planetSystems->getClosestSystemPosition())
	{
		loadClosestSystem();
	}

	lastData   = planetSystems->getClosestSystemPosition();
	sysInWorld = camCosmo.dataToWorldPosition(lastData);

	CelestialBodyRenderer::overridenScale = mtokpc * camCosmo.scale;

	if((camPlanet.target == orbitalSystem->getMainCelestialBody()
	    && CelestialBodyRenderer::overridenScale < 1e-12)
	   || forceUpdateFromCosmo)
	{
		camPlanet.relativePosition
		    = -1 * sysInWorld / (mtokpc * camCosmo.scale);
		forceUpdateFromCosmo = false;
	}
	sysInWorld = camCosmo.dataToWorldPosition(lastData);
	planetSystems->getClosestSystem()->update(currentUt);
	systemRenderer->updateMesh(currentUt, camPlanet);
}

void Universe::renderCosmo(ToneMappingModel const& toneMappingModel)
{
	GLHandler::glf().glDepthFunc(GL_LEQUAL);
	GLHandler::glf().glEnable(GL_DEPTH_CLAMP);
	GLHandler::glf().glEnable(GL_CLIP_DISTANCE0);
	for(auto pair : elements)
	{
		// only used by CosmologicalLabels for now
		// pair.second->setVisibility(CelestialBodyRenderer::renderLabels);
		if(pair.second->getVisibility() < 0.001)
		{
			continue;
		}
		pair.second->render(camCosmo, toneMappingModel);
	}

	planetSystems->render(camCosmo, toneMappingModel);

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
}

Universe::~Universe()
{
	delete systemRenderer;
	for(auto const& pair : elements)
	{
		delete pair.second;
	}
}
