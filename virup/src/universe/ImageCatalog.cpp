/*
    Copyright (C) 2025 Florian Cabot <florian.cabot@hotmail.fr>

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

#include "universe/ImageCatalog.hpp"

#include "Primitives.hpp"
#include "paint/AdvancedPainter.hpp"

ImageCatalog::ImageCatalog()
    : shader("imgcatalog")
{
	Primitives::setAsQuad(quad, shader);
}

QJsonObject ImageCatalog::getJson() const
{
	auto result(UniverseElement::getJson());
	result["type"] = "imgcatalog";
	result["file"] = file;
	return result;
}

void ImageCatalog::setJson(QJsonObject const& json)
{
	UniverseElement::setJson(json);
	file = json["file"].toString();

	tex = std::make_unique<GLTexture>(
	    (QSettings().value("data/rootdir").toString() + file)
	        .toLatin1()
	        .data());
	tex->setSampler(GLTexture::Sampler{GL_LINEAR, GL_CLAMP_TO_BORDER});
	tex->setBorderColor(Qt::black); // forces alpha = 1.0

	texAspectRatio = static_cast<float>(tex->getSize()[0]) / tex->getSize()[1];
}

void ImageCatalog::renderGui(QSize const& targetSize,
                             AdvancedPainter& /*painter*/)
{
	QVector2D scale(1.f, 1.f);

	float targetAspectRatio(static_cast<float>(targetSize.width())
	                        / targetSize.height());

	if(targetAspectRatio > texAspectRatio)
	{
		scale.setX(targetAspectRatio / texAspectRatio);
	}
	else
	{
		scale.setY(texAspectRatio / targetAspectRatio);
	}

	shader.setUniform("scale", scale);
	shader.setUniform("alpha", getVisibility());

	const GLBlendSet glBlend(GLBlendSet::BlendState{});
	const GLStateSet glState({{GL_CULL_FACE, false}});

	GLHandler::useTextures({tex.get()});
	GLHandler::setUpRender(shader);
	quad.render();
}

QList<QPair<QString, QWidget*>>
    ImageCatalog::getLauncherFields(QWidget& parent, QJsonObject& jsonObj)
{
	QList<QPair<QString, QWidget*>> result;

	auto* pathSelector
	    = make_qt_unique<PathSelector>(parent, QObject::tr("Textures path"));
	QObject::connect(pathSelector, &PathSelector::pathChanged,
	                 [&jsonObj](QString const& path)
	                 { jsonObj["file"] = path; });
	pathSelector->setPath(jsonObj["file"].toString());

	result.append({QObject::tr("Textures Path:"), pathSelector});

	return result;
}
