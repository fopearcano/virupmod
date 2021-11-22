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

#include "CosmologicalSimulation.hpp"

float& CosmologicalSimulation::animationTime()
{
	static float animationTime(0.f);
	return animationTime;
}

CosmologicalSimulation::CosmologicalSimulation(QJsonObject const& json)
{
	QString rootdir(QSettings().value("data/rootdir").toString() + '/'),
	    gasPath(json["gasfile"].toString()),
	    starsPath(json["starsfile"].toString()),
	    dmPath(json["darkmatterfile"].toString());
	if(!gasPath.isEmpty())
	{
		gasPath = rootdir + gasPath;
	}
	if(!starsPath.isEmpty())
	{
		starsPath = rootdir + starsPath;
	}
	if(!dmPath.isEmpty())
	{
		dmPath = rootdir + dmPath;
	}

	init(gasPath.toStdString(), starsPath.toStdString(),
	     json["loaddarkmatter"].toBool() ? dmPath.toStdString() : "",
	     json["gascolor"].toString(), json["starscolor"].toString(),
	     json["darkmattercolor"].toString());
}

CosmologicalSimulation::CosmologicalSimulation(
    std::string const& gasOctreePath, std::string const& starsOctreePath,
    std::string const& darkMatterOctreePath, bool loadDarkMatter,
    QColor const& gasColor, QColor const& starsColor,
    QColor const& darkMatterColor)
{
	init(gasOctreePath, starsOctreePath,
	     loadDarkMatter ? darkMatterOctreePath : "", gasColor, starsColor,
	     darkMatterColor);
}

void CosmologicalSimulation::init(std::string const& gasOctreePath,
                                  std::string const& starsOctreePath,
                                  std::string const& darkMatterOctreePath,
                                  QColor const& gasColor,
                                  QColor const& starsColor,
                                  QColor const& darkMatterColor)
{
	QRegularExpression rxNumber("[0-9]+");
	QString dirPathGas = gasOctreePath.c_str();
	QDir gasDir(dirPathGas);
	if(!gasOctreePath.empty())
	{
		for(auto const& path :
		    gasDir.entryList({"*.dat", "*.octree"}, QDir::Files, QDir::Name))
		{
			unsigned int index(rxNumber.match(path).captured().toInt());
			cosmoFilesGas[index] = dirPathGas + "/" + path;
		}
	}

	QString dirPathStars = starsOctreePath.c_str();
	QDir starsDir(dirPathStars);
	for(auto const& path :
	    starsDir.entryList({"*.octree"}, QDir::Files, QDir::Name))
	{
		unsigned int index(rxNumber.match(path).captured().toInt());
		cosmoFilesStars[index] = dirPathStars + "/" + path;
	}

	QString dirPathDM = darkMatterOctreePath.c_str();
	QDir dmDir(dirPathDM);
	if(!darkMatterOctreePath.empty())
	{
		for(auto const& path :
		    dmDir.entryList({"*.dat", "*.octree"}, QDir::Files, QDir::Name))
		{
			unsigned int index(rxNumber.match(path).captured().toInt());
			cosmoFilesDM[index] = dirPathDM + "/" + path;
		}
	}

	if(cosmoFilesGas.empty())
	{
		cosmoFilesGas[0] = gasOctreePath.c_str();
	}
	if(cosmoFilesStars.empty())
	{
		cosmoFilesStars[0] = starsOctreePath.c_str();
	}
	if(cosmoFilesDM.empty())
	{
		cosmoFilesDM[0] = darkMatterOctreePath.c_str();
	}

	maxIndex = cosmoFilesGas.end()->first;
	maxIndex = cosmoFilesStars.end()->first > maxIndex
	               ? cosmoFilesStars.end()->first
	               : maxIndex;
	maxIndex = cosmoFilesDM.end()->first > maxIndex ? cosmoFilesDM.end()->first
	                                                : maxIndex;

	this->gasColor        = gasColor;
	this->starsColor      = starsColor;
	this->darkMatterColor = darkMatterColor;
	trees.init(cosmoFilesGas[0].toStdString(), cosmoFilesStars[0].toStdString(),
	           cosmoFilesDM[0].toStdString());
	trees.setColors(gasColor, starsColor, darkMatterColor);

	trees.silent = true;
}

unsigned int CosmologicalSimulation::getClosestId(
    std::map<unsigned int, QString> const& m, unsigned index)
{
	if(m.count(index) > 0)
	{
		return index;
	}
	unsigned int l(m.lower_bound(index)->first);
	unsigned int u(m.upper_bound(index)->first);
	if(index - l < u - index)
	{
		return l;
	}
	return u;
}

BBox CosmologicalSimulation::getBoundingBox() const
{
	return trees.getDataBoundingBox();
}

uint64_t CosmologicalSimulation::getOctreesTotalDataSize() const
{
	return trees.getOctreesTotalDataSize();
}

bool CosmologicalSimulation::preloadOctreesLevel(unsigned int level,
                                                 QProgressDialog* progress)
{
	return trees.preloadOctreesLevel(level, progress);
}

void CosmologicalSimulation::update(Camera const& camera)
{
	getModelAndCampos(camera, model, campos);

	if(cosmoFilesGas.size() > 1 || cosmoFilesStars.size() > 1
	   || cosmoFilesDM.size() > 1)
	{
		auto oldCurrent(currentIndex);
		currentIndex
		    = static_cast<unsigned int>(animationTime() * (maxIndex - 1));
		if(currentIndex != oldCurrent)
		{
			trees.cleanUp();
			trees.init(
			    cosmoFilesGas[getClosestId(cosmoFilesGas, currentIndex)]
			        .toStdString(),
			    cosmoFilesStars[getClosestId(cosmoFilesStars, currentIndex)]
			        .toStdString(),
			    cosmoFilesDM[getClosestId(cosmoFilesDM, currentIndex)]
			        .toStdString());
			trees.setColors(gasColor, starsColor, darkMatterColor);
		}
	}

	trees.update(camera, model, campos);
}

void CosmologicalSimulation::render(Camera const& camera,
                                    ToneMappingModel const& /*tmm*/)
{
	trees.setAlpha(visibility * brightnessMultiplier);
	GLHandler::glf().glEnable(GL_CLIP_DISTANCE0);
	trees.render(camera, model, campos, unit);
	GLHandler::glf().glDisable(GL_CLIP_DISTANCE0);
}

QList<QPair<QString, QWidget*>>
    CosmologicalSimulation::getLauncherFields(QWidget* parent,
                                              QJsonObject* jsonObj)
{
	QList<QPair<QString, QWidget*>> result;

	auto pathSelector = new PathSelector(parent, QObject::tr("Gas path"));
	QObject::connect(
	    pathSelector, &PathSelector::pathChanged,
	    [jsonObj](QString const& path) { (*jsonObj)["gasfile"] = path; });
	pathSelector->setPath((*jsonObj)["gasfile"].toString());

	result.append({QObject::tr("Gas Path:"), pathSelector});

	pathSelector = new PathSelector(parent, QObject::tr("Stars path"));
	QObject::connect(
	    pathSelector, &PathSelector::pathChanged,
	    [jsonObj](QString const& path) { (*jsonObj)["starsfile"] = path; });
	pathSelector->setPath((*jsonObj)["starsfile"].toString());

	result.append({QObject::tr("Stars Path:"), pathSelector});

	pathSelector = new PathSelector(parent, QObject::tr("Dark matter path"));
	QObject::connect(pathSelector, &PathSelector::pathChanged,
	                 [jsonObj](QString const& path) {
		                 (*jsonObj)["darkmatterfile"] = path;
	                 });
	pathSelector->setPath((*jsonObj)["darkmatterfile"].toString());

	result.append({QObject::tr("Dark Matter Path:"), pathSelector});

	auto cbox = new QCheckBox(parent);
	QObject::connect(cbox, &QCheckBox::stateChanged, [jsonObj](int state) {
		(*jsonObj)["loaddarkmatter"] = (state == Qt::Checked);
	});
	cbox->setCheckState((*jsonObj)["loaddarkmatter"].toBool() ? Qt::Checked
	                                                          : Qt::Unchecked);

	result.append({QObject::tr("Load Dark Matter:"), cbox});

	auto colorSelector = new ColorSelector(parent, QObject::tr("Gas color"));
	QObject::connect(colorSelector, &ColorSelector::colorChanged,
	                 [jsonObj](QColor const& color) {
		                 (*jsonObj)["gascolor"] = color.name();
	                 });
	colorSelector->setColor((*jsonObj)["gascolor"].toString("#000000"));

	result.append({QObject::tr("Gas Color:"), colorSelector});

	colorSelector = new ColorSelector(parent, QObject::tr("Stars color"));
	QObject::connect(colorSelector, &ColorSelector::colorChanged,
	                 [jsonObj](QColor const& color) {
		                 (*jsonObj)["starscolor"] = color.name();
	                 });
	colorSelector->setColor((*jsonObj)["starscolor"].toString("#000000"));

	result.append({QObject::tr("Stars Color:"), colorSelector});

	colorSelector = new ColorSelector(parent, QObject::tr("Dark matter color"));
	QObject::connect(colorSelector, &ColorSelector::colorChanged,
	                 [jsonObj](QColor const& color) {
		                 (*jsonObj)["darkmattercolor"] = color.name();
	                 });
	colorSelector->setColor((*jsonObj)["darkmattercolor"].toString("#000000"));

	result.append({QObject::tr("Dark Matter Color:"), colorSelector});

	return result;
}

void CosmologicalSimulation::dumpOctreesStates(QString const& dirPath,
                                               QString const& filePathPrefix)
{
	trees.dumpOctreesStates(dirPath, filePathPrefix);
}
