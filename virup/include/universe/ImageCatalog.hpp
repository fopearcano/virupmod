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

  private:
	GLShaderProgram shader;
	GLMesh quad;
	std::unique_ptr<GLTexture> tex;
	float texAspectRatio = 0.f;

	QString dir;
	QString currentImage;

	QStringList files;
};

#endif // IMAGECATALOG_HPP
