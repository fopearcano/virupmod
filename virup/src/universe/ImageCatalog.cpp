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

// UTILS
QByteArray escapeInQuotes(const QByteArray& input)
{
	QByteArray result;
	bool inQuotes = false;

	for(int i = 0; i < input.size(); ++i)
	{
		const char c = input[i];

		if(c == '"')
		{
			// Toggle quote state unless escaped
			const bool escaped = (i > 0 && input[i - 1] == '\\');
			if(!escaped)
			{
				inQuotes = !inQuotes;
			}
			result.append(c);
		}
		else if(inQuotes)
		{
			if(c == '\n')
			{
				result.append("\\n");
			}
			else if(c == ',')
			{
				result.append("&#44;");
			}
			else
			{
				result.append(c);
			}
		}
		else
		{
			result.append(c);
		}
	}

	return result;
}
// END UTILS

ImageCatalog::ImageCatalog()
    : shader("imgcatalog")
{
	Primitives::setAsQuad(quad, shader);
}

QJsonObject ImageCatalog::getJson() const
{
	auto result(UniverseElement::getJson());
	result["type"]     = "imgcatalog";
	result["metadata"] = metadata;
	result["dir"]      = dir;
	return result;
}

void ImageCatalog::setJson(QJsonObject const& json)
{
	metadataHeader.clear();
	metadataContent.clear();
	idColumn = -1;
	files.clear();

	UniverseElement::setJson(json);

	const QString rootdir(QSettings().value("data/rootdir").toString());

	// load metadta
	metadata = json["metadata"].toString();

	{
		QFile metadataFile(rootdir + '/' + metadata);
		metadataFile.open(QIODevice::ReadOnly);
		metadataHeader = QString{metadataFile.readLine()}.trimmed().split(',');

		auto wholeDoc = metadataFile.readAll();
		wholeDoc      = escapeInQuotes(wholeDoc);
		for(auto const& line : QString{wholeDoc}.split('\n'))
		{
			metadataContent.append(line.trimmed().split(','));
		}
		// look for "id" column
		int i = 0;
		for(auto const& columnHeader : metadataHeader)
		{
			if(columnHeader.toLower() == "id")
			{
				idColumn = i;
			}
			if(columnHeader.toLower() == "title")
			{
				titleColumn = i;
			}
			if(columnHeader.toLower() == "description")
			{
				descriptionColumn = i;
			}
			++i;
		}
		if(idColumn == -1)
		{
			qWarning() << "Image Catalog metadata" << json["name"].toString()
			           << "doesn't have an id column.";
		}
		if(titleColumn == -1)
		{
			qWarning() << "Image Catalog metadata" << json["name"].toString()
			           << "doesn't have a title column.";
		}
		if(descriptionColumn == -1)
		{
			qWarning() << "Image Catalog metadata" << json["name"].toString()
			           << "doesn't have a description column.";
		}
	}

	// load images
	dir = json["dir"].toString() + '/';

	const QDir directory(rootdir + '/' + dir + '/');
	QStringList nameFilters;
	nameFilters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.gif"
	            << "*.tiff" << "*.webp";

	auto fileInfos = directory.entryInfoList(nameFilters, QDir::Files);

	for(const QFileInfo& fileInfo : fileInfos)
	{
		files << fileInfo.fileName();
	}

	for(auto const& file : files)
	{
		auto id = file.split('.').first();
		for(int i(0); i < metadataContent.size(); ++i)
		{
			if(metadataContent[i].at(idColumn) == id)
			{
				map[file] = i;
			}
		}
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
	                 Qt::AlignHCenter | Qt::AlignTop,
	                 getTitleDescription(currentImage).first);
}

QList<QPair<QString, QWidget*>>
    ImageCatalog::getLauncherFields(QWidget& parent, QJsonObject& jsonObj)
{
	QList<QPair<QString, QWidget*>> result;

	auto* metadataPathSelector
	    = make_qt_unique<PathSelector>(parent, QObject::tr("Metadata file"));
	QObject::connect(metadataPathSelector, &PathSelector::pathChanged,
	                 [&jsonObj](QString const& path)
	                 { jsonObj["metadata"] = path; });
	metadataPathSelector->setPath(jsonObj["metadata"].toString());

	auto* pathSelector = make_qt_unique<PathSelector>(
	    parent, QObject::tr("Textures directory"),
	    PathSelector::Type::DIRECTORY);
	QObject::connect(pathSelector, &PathSelector::pathChanged,
	                 [&jsonObj](QString const& path)
	                 { jsonObj["dir"] = path; });
	pathSelector->setPath(jsonObj["dir"].toString());

	result.append({QObject::tr("Metadata File:"), metadataPathSelector});
	result.append({QObject::tr("Textures Directory:"), pathSelector});

	return result;
}

void ImageCatalog::setImage(QString const& image)
{
	currentImage = image;

	const QString rootdir(QSettings().value("data/rootdir").toString());
	tex = std::make_unique<GLTexture>(
	    (rootdir + '/' + dir + image).toLatin1().data(), false);
	tex->setSampler(GLTexture::Sampler{GL_LINEAR, GL_CLAMP_TO_BORDER});
	tex->setBorderColor(Qt::black); // forces alpha = 1.0

	texAspectRatio = static_cast<float>(tex->getSize()[0]) / tex->getSize()[1];
}

QPair<QString, QString>
    ImageCatalog::getTitleDescription(QString const& file) const
{
	if(map.contains(file))
	{
		auto title = metadataContent[map[file]].at(titleColumn);
		title.replace("&#44;", ",");
		title.replace("\\n", "\n");
		auto description = metadataContent[map[file]].at(descriptionColumn);
		description.replace("&#44;", ",");
		description.replace("\\n", "\n");
		return {title, description};
	}

	return {file, ""};
}
