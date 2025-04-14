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
	result["dir"]  = dir;
	return result;
}

void ImageCatalog::setJson(QJsonObject const& json)
{
	files.clear();

	UniverseElement::setJson(json);
	dir = json["dir"].toString() + '/';

	const QString rootdir(QSettings().value("data/rootdir").toString());
	const QDir directory(rootdir + '/' + dir + '/');
	QStringList nameFilters;
	nameFilters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.gif"
	            << "*.tiff" << "*.webp";

	auto fileInfos = directory.entryInfoList(nameFilters, QDir::Files);

	for(const QFileInfo& fileInfo : fileInfos)
	{
		files << fileInfo.fileName();
	}

	setImage(files.first());
}

void ImageCatalog::renderGui(QSize const& targetSize, AdvancedPainter& painter)
{
	QVector2D scale(1.f, 1.f);

	const float targetAspectRatio(static_cast<float>(targetSize.width())
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

	const QPen pen(Qt::red);
	QFont font = painter.font();
	font.setPointSizeF(24.f * targetSize.height() / 1080);

	painter.setPen(pen);
	painter.setFont(font);
	painter.drawText(0, 0, targetSize.width(), targetSize.height(),
	                 Qt::AlignHCenter | Qt::AlignTop, currentImage);
}

QList<QPair<QString, QWidget*>>
    ImageCatalog::getLauncherFields(QWidget& parent, QJsonObject& jsonObj)
{
	QList<QPair<QString, QWidget*>> result;

	auto* pathSelector = make_qt_unique<PathSelector>(
	    parent, QObject::tr("Textures directory"),
	    PathSelector::Type::DIRECTORY);
	QObject::connect(pathSelector, &PathSelector::pathChanged,
	                 [&jsonObj](QString const& path)
	                 { jsonObj["dir"] = path; });
	pathSelector->setPath(jsonObj["dir"].toString());

	result.append({QObject::tr("Textures Directory:"), pathSelector});

	return result;
}

void ImageCatalog::setImage(QString const& image)
{
	currentImage = image;

	const QString rootdir(QSettings().value("data/rootdir").toString());
	tex = std::make_unique<GLTexture>(
	    (rootdir + '/' + dir + image).toLatin1().data());
	tex->setSampler(GLTexture::Sampler{GL_LINEAR, GL_CLAMP_TO_BORDER});
	tex->setBorderColor(Qt::black); // forces alpha = 1.0

	texAspectRatio = static_cast<float>(tex->getSize()[0]) / tex->getSize()[1];
}
