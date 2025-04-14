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

#ifndef IMAGECATALOG_HPP
#define IMAGECATALOG_HPP

#include "universe/UniverseElement.hpp"

class ImageCatalog : public UniverseElement
{
  public:
	ImageCatalog();
	virtual QJsonObject getJson() const override;
	virtual void setJson(QJsonObject const& json) override;
	virtual BBox getBoundingBox() const override { return {}; };
	void renderGui(QSize const& targetSize, AdvancedPainter& painter) override;

	static QList<QPair<QString, QWidget*>>
	    getLauncherFields(QWidget& parent, QJsonObject& jsonObj);

	QStringList const& getFiles() const { return files; };
	QString getImage() const { return currentImage; };
	void setImage(QString const& image);

	QPair<QString, QString> getTitleDescription(QString const& file) const;

  private:
	GLShaderProgram shader;
	GLMesh quad;
	std::unique_ptr<GLTexture> tex;
	float texAspectRatio = 0.f;

	QString metadata;
	QString dir;

	QStringList metadataHeader;
	QList<QStringList> metadataContent;
	int idColumn          = -1;
	int titleColumn       = -1;
	int descriptionColumn = -1;

	QString currentImage;

	QStringList files;

	// maps fileName to metadataContent entry
	QMap<QString, int> map;
};

#endif // IMAGECATALOG_HPP
