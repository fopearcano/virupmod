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

#ifndef COSMOLOGICALSIMULATION_HPP
#define COSMOLOGICALSIMULATION_HPP

#include <QCheckBox>
#include <map>
#include <set>

#include "UniverseElement.hpp"
#include "methods/TreeMethodLOD.hpp"

class CosmologicalSimulation : public UniverseElement
{
  public:
	CosmologicalSimulation(QJsonObject const& json);
	CosmologicalSimulation(std::string const& gasOctreePath,
	                       std::string const& starsOctreePath,
	                       std::string const& darkMatterOctreePath,
	                       bool loadDarkMatter, QColor const& gasColor,
	                       QColor const& starsColor,
	                       QColor const& darkMatterColor);
	virtual BBox getBoundingBox() const override;
	uint64_t getOctreesTotalDataSize() const;
	bool preloadOctreesLevel(unsigned int level, QProgressDialog& progress);
	virtual void update(Camera const& camera) override;
	virtual void render(Camera const& camera,
	                    ToneMappingModel const& tmm) override;
	~CosmologicalSimulation() = default;

	static QList<QPair<QString, QWidget*>>
	    getLauncherFields(QWidget* parent, QJsonObject* jsonObj);

  public:
	void init(std::string const& gasOctreePath,
	          std::string const& starsOctreePath,
	          std::string const& darkMatterOctreePath, QColor const& gasColor,
	          QColor const& starsColor, QColor const& darkMatterColor);
	TreeMethodLOD trees;

	QMatrix4x4 model;
	QVector3D campos;

	std::map<unsigned int, QString> cosmoFilesGas;
	std::map<unsigned int, QString> cosmoFilesStars;
	std::map<unsigned int, QString> cosmoFilesDM;
	unsigned int currentIndex = 0;
	unsigned int maxIndex     = 0;

	QColor gasColor;
	QColor starsColor;
	QColor darkMatterColor;
};

#endif // COSMOLOGICALSIMULATION_HPP
