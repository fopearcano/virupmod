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

#include "TexturedSphere.hpp"

TexturedSphere::TexturedSphere(QJsonObject const& json)
    : shader("texturedsphere")
    , tex(json["file"].toString().toLatin1().data())
    , cullFrontFaces(json["cullfrontfaces"].toBool())
{
	Primitives::setAsUnitSphere(mesh, shader, 50, 50);
}

void TexturedSphere::render(Camera const& camera,
                            ToneMappingModel const& /*tmm*/)
{
	QMatrix4x4 model;
	QVector3D campos;
	getModelAndCampos(camera, model, campos);

	shader.setUniform("exposure", visibility * brightnessMultiplier);

	GLHandler::beginTransparent(GL_ONE, GL_ONE);
	GLHandler::setBackfaceCulling(cullFrontFaces, GL_FRONT);
	GLHandler::useTextures({&tex});
	GLHandler::setUpRender(shader, model);
	mesh.render();
	GLHandler::setBackfaceCulling(true);
	GLHandler::endTransparent();
}

QList<QPair<QString, QWidget*>>
    TexturedSphere::getLauncherFields(QWidget* parent, QJsonObject* jsonObj)
{
	QList<QPair<QString, QWidget*>> result;

	auto pathSelector = new PathSelector(parent, QObject::tr("Texture path"));
	QObject::connect(
	    pathSelector, &PathSelector::pathChanged,
	    [jsonObj](QString const& path) { (*jsonObj)["file"] = path; });
	pathSelector->setPath((*jsonObj)["file"].toString());

	result.append({QObject::tr("Texture Path:"), pathSelector});

	auto cbox = new QCheckBox(parent);
	QObject::connect(cbox, &QCheckBox::stateChanged, [jsonObj](int state) {
		(*jsonObj)["cullfrontfaces"] = (state == Qt::Checked);
	});
	cbox->setCheckState((*jsonObj)["cullfrontfaces"].toBool() ? Qt::Checked
	                                                          : Qt::Unchecked);

	result.append({QObject::tr("Cull front faces :"), cbox});

	return result;
}
