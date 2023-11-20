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

#include "universe/TexturedSphere.hpp"

TexturedSphere::TexturedSphere()
    : shader("texturedsphere")
{
	Primitives::setAsUnitSphere(mesh, shader, 50, 50);
}

QJsonObject TexturedSphere::getJson() const
{
	auto result(UniverseElement::getJson());
	result["type"]           = "texsphere";
	result["file"]           = file;
	result["cullfrontfaces"] = cullFrontFaces;
	return result;
}

void TexturedSphere::setJson(QJsonObject const& json)
{
	UniverseElement::setJson(json);
	file           = json["file"].toString();
	cullFrontFaces = json["cullfrontfaces"].toBool();

	qDebug() << QSettings().value("data/rootdir").toString() + file;
	tex = std::make_unique<GLTexture>(
	    (QSettings().value("data/rootdir").toString() + file)
	        .toLatin1()
	        .data());
}

BBox TexturedSphere::getBoundingBox() const
{
	return {-1.0, 1.0, -1.0, 1.0, -1.0, 1.0, 2.0, {0.0, 0.0, 0.0}};
}

void TexturedSphere::render(Camera const& camera,
                            ToneMappingModel const& /*tmm*/)
{
	QMatrix4x4 model;
	QVector3D campos;
	getModelAndCampos(camera, model, campos);

	auto vis = getVisibility();
	if(useBrightnessMultiplier())
	{
		vis *= brightnessMultiplier;
	}
	shader.setUniform("exposure", vis);

	GLBlendSet glBlend({GL_ONE, GL_ONE});
	GLHandler::setBackfaceCulling(cullFrontFaces, GL_FRONT);
	GLHandler::useTextures({tex.get()});
	GLHandler::setUpRender(shader, model);
	mesh.render();
	GLHandler::setBackfaceCulling(true);
}

QList<QPair<QString, QWidget*>>
    TexturedSphere::getLauncherFields(QWidget& parent, QJsonObject& jsonObj)
{
	QList<QPair<QString, QWidget*>> result;

	auto pathSelector
	    = make_qt_unique<PathSelector>(parent, QObject::tr("Texture path"));
	QObject::connect(pathSelector, &PathSelector::pathChanged,
	                 [&jsonObj](QString const& path)
	                 { jsonObj["file"] = path; });
	pathSelector->setPath(jsonObj["file"].toString());

	result.append({QObject::tr("Texture Path:"), pathSelector});

	auto cbox = make_qt_unique<QCheckBox>(parent);
	QObject::connect(cbox, &QCheckBox::stateChanged,
	                 [&jsonObj](int state)
	                 { jsonObj["cullfrontfaces"] = (state == Qt::Checked); });
	cbox->setCheckState(jsonObj["cullfrontfaces"].toBool() ? Qt::Checked
	                                                       : Qt::Unchecked);

	result.append({QObject::tr("Cull front faces :"), cbox});

	return result;
}
