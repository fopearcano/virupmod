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

#include "universe/Credits.hpp"

Credits::Credits()
    : shader("credits")
{
	Primitives::setAsUnitCube(mesh, shader);
}

QJsonObject Credits::getJson() const
{
	auto result(UniverseElement::getJson());
	result["type"] = "credits";
	result["file"] = file;
	return result;
}

void Credits::setJson(QJsonObject const& json)
{
	UniverseElement::setJson(json);
	file = json["file"].toString();

	tex = std::make_unique<GLTexture>(
	    (QSettings().value("data/rootdir").toString() + file)
	        .toLatin1()
	        .data());
}

void Credits::render(Camera const& camera, ToneMappingModel const& tmm)
{
	shader.setUniform("exposure", tmm.exposure);
	shader.setUniform("dynamicrange", tmm.dynamicrange);
	shader.setUniform("alpha", 1.f); // visibility * brightnessMultiplier);
	shader.setUniform("color", QVector3D(10000.0, 0.0, 0.0));

	auto size = tex->getSize();
	shader.setUniform("aspectratio", size[0] / size[1]);

	// Conserve credits center position and Keep parallel to view "up"
	QMatrix4x4 model;
	model.translate({-0.5f, 0.f, 0.f});
	model.rotate(camera.pitch * 45.f / M_PI, {0.f, 1.f, 0.f});
	model.translate({0.5f, 0.f, 0.f});

	GLStateSet glState({{GL_CULL_FACE, false}});
	GLHandler::useTextures({tex.get()});
	GLHandler::setUpRender(shader, model);
	mesh.render(PrimitiveType::TRIANGLE_STRIP);
}

QList<QPair<QString, QWidget*>> Credits::getLauncherFields(QWidget& parent,
                                                           QJsonObject& jsonObj)
{
	QList<QPair<QString, QWidget*>> result;

	auto pathSelector
	    = make_qt_unique<PathSelector>(parent, QObject::tr("Textures path"));
	QObject::connect(pathSelector, &PathSelector::pathChanged,
	                 [&jsonObj](QString const& path)
	                 { jsonObj["file"] = path; });
	pathSelector->setPath(jsonObj["file"].toString());

	result.append({QObject::tr("Textures Path:"), pathSelector});

	return result;
}
