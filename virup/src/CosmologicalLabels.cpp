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

#include "CosmologicalLabels.hpp"

CosmologicalLabels::CosmologicalLabels(QJsonObject const& json)
{
	QString path(json["file"].toString());
	visibility = 0.f;
	QFile f(path);
	if(!f.open(QFile::ReadOnly | QFile::Text))
	{
		std::cerr << "Invalid cosmological labels file path : "
		          << path.toStdString() << std::endl;
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

			auto labelText
			    = new LabelRenderer(label, QColor(json["color"].toString()));
			cosmoLabels.emplace_back(dataPos, labelText);
		}
	}
}

void CosmologicalLabels::update(Camera const& camera)
{
	getModelAndCampos(camera, model, campos);

	Vector3 camPosData(camera.worldToDataPosition(Utils::fromQt(
	    camera.hmdScaledSpaceToWorldTransform() * QVector3D(0.f, 0.f, 0.f))));

	for(auto cosmoLabel : cosmoLabels)
	{
		Vector3 posData = Utils::fromQt(this->getRelToAbsTransform()
		                                * Utils::toQt(cosmoLabel.first));
		Vector3 pos(camera.dataToWorldPosition(posData));
		Vector3 camRelPos(camPosData - posData);
		Vector3 unitRelPos(camRelPos.getUnitForm());

		float yaw(atan2(unitRelPos[1], unitRelPos[0]));
		float pitch(-1.0 * asin(unitRelPos[2]));
		double rescale(pos.length() <= 8000.0 ? 1.0 : 8000.0 / pos.length());
		QMatrix4x4 model;
		model.translate(Utils::toQt(pos * rescale));
		model.scale(rescale * camRelPos.length() * camera.scale / 3.0);
		model.rotate(yaw * 180.f / M_PI + 90.f, 0.0, 0.0, 1.0);
		model.rotate(pitch * 180.f / M_PI + 90.f, 1.0, 0.0, 0.0);
		cosmoLabel.second->updateModel(model);
	}
}

void CosmologicalLabels::render(Camera const& /*camera*/,
                                ToneMappingModel const& tmm)
{
	if(visibility > 0.f)
	{
		for(auto cosmoLabel : cosmoLabels)
		{
			/*Vector3 posData = Utils::fromQt(this->getRelToAbsTransform()
			                                * Utils::toQt(cosmoLabel.first));
			if(posData == solarSystemDataPos && planetSystems->renderSystem()
			   && planetSystems->getClosestSystem()->getName()
			          == "Solar System")
			{
			    continue;
			}*/
			cosmoLabel.second->setAlpha(visibility);
			cosmoLabel.second->render(tmm.exposure, tmm.dynamicrange);
		}
	}
}

CosmologicalLabels::~CosmologicalLabels()
{
	for(auto cosmoLabel : cosmoLabels)
	{
		delete cosmoLabel.second;
	}
}

QList<QPair<QString, QWidget*>>
    CosmologicalLabels::getLauncherFields(QWidget* parent, QJsonObject* jsonObj)
{
	QList<QPair<QString, QWidget*>> result;

	auto pathSelector = new PathSelector(parent, QObject::tr("File path"));
	QObject::connect(
	    pathSelector, &PathSelector::pathChanged,
	    [jsonObj](QString const& path) { (*jsonObj)["file"] = path; });
	pathSelector->setPath((*jsonObj)["file"].toString());

	result.append({QObject::tr("File Path:"), pathSelector});

	auto colorSelector = new ColorSelector(parent, QObject::tr("Color"));
	QObject::connect(
	    colorSelector, &ColorSelector::colorChanged,
	    [jsonObj](QColor const& color) { (*jsonObj)["color"] = color.name(); });
	colorSelector->setColor((*jsonObj)["color"].toString("#FF0000"));

	result.append({QObject::tr("Color:"), colorSelector});

	return result;
}
