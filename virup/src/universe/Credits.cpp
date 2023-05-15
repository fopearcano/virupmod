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

	delete tex;
	tex = new GLTexture((QSettings().value("data/rootdir").toString() + file)
	                        .toLatin1()
	                        .data());
}

void Credits::render(Camera const& /*camera*/, ToneMappingModel const& tmm)
{
	shader.setUniform("exposure", tmm.exposure);
	shader.setUniform("dynamicrange", tmm.dynamicrange);
	shader.setUniform("alpha", 1.f); // visibility * brightnessMultiplier);
	shader.setUniform("color", QVector3D(10000.0, 0.0, 0.0));

	auto size = tex->getSize();
	shader.setUniform("aspectratio", size.width() / size.height());

	QMatrix4x4 scale;
	scale.scale(1.f);

	GLHandler::setBackfaceCulling(false);
	GLHandler::useTextures({tex});
	GLHandler::setUpRender(shader, scale);
	mesh.render(PrimitiveType::TRIANGLE_STRIP);
	GLHandler::setBackfaceCulling(true);
}

QList<QPair<QString, QWidget*>> Credits::getLauncherFields(QWidget* parent,
                                                           QJsonObject* jsonObj)
{
	QList<QPair<QString, QWidget*>> result;

	auto pathSelector = new PathSelector(parent, QObject::tr("Textures path"));
	QObject::connect(pathSelector, &PathSelector::pathChanged,
	                 [jsonObj](QString const& path)
	                 { (*jsonObj)["file"] = path; });
	pathSelector->setPath((*jsonObj)["file"].toString());

	result.append({QObject::tr("Textures Path:"), pathSelector});

	return result;
}

Credits::~Credits()
{
	delete tex;
}
