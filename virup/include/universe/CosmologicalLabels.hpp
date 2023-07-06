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

#ifndef COSMOLOGICALLABELS_HPP
#define COSMOLOGICALLABELS_HPP

#include "graphics/renderers/LabelRenderer.hpp"
#include "universe/UniverseElement.hpp"

class CosmologicalLabels : public UniverseElement
{
  public:
	CosmologicalLabels();
	virtual QJsonObject getJson() const override;
	virtual void setJson(QJsonObject const& json) override;
	virtual BBox getBoundingBox() const override { return bbox; };
	virtual void update(Camera const& camera) override;
	virtual void render(Camera const& camera,
	                    ToneMappingModel const& tmm) override;

	static QList<QPair<QString, QWidget*>>
	    getLauncherFields(QWidget& parent, QJsonObject& jsonObj);

  private:
	// in kpc
	std::vector<std::pair<Vector3, LabelRenderer>> cosmoLabels;

	QMatrix4x4 model;
	QVector3D campos;

	BBox bbox;

	QString file;
	QColor color;
};

#endif // COSMOLOGICALLABELS_HPP
