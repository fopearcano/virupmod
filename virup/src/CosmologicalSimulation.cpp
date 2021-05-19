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

CosmologicalSimulation::CosmologicalSimulation(QJsonObject const& json)
{
	trees.setColors(json["gascolor"].toString(), json["starscolor"].toString(),
	                json["darkmattercolor"].toString());
	trees.init(json["gasfile"].toString().toStdString(),
	           json["starsfile"].toString().toStdString(),
	           json["loaddarkmatter"].toBool()
	               ? json["darkmatterfile"].toString().toStdString()
	               : "");
}

CosmologicalSimulation::CosmologicalSimulation(
    std::string const& gazOctreePath, std::string const& starsOctreePath,
    std::string const& darkMatterOctreePath)
{
	trees.init(gazOctreePath, starsOctreePath, darkMatterOctreePath);
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
                                                 QProgressDialog& progress)
{
	return trees.preloadOctreesLevel(level, progress);
}

void CosmologicalSimulation::update(Camera const& camera)
{
	getModelAndCampos(camera, model, campos);

	trees.update(camera, model, campos);
}

void CosmologicalSimulation::render(Camera const& camera,
                                    ToneMappingModel const& /*tmm*/)
{
	trees.setAlpha(brightnessMultiplier);
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
