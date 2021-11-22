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
	bool preloadOctreesLevel(unsigned int level,
	                         QProgressDialog* progress = nullptr);
	virtual void update(Camera const& camera) override;
	virtual void render(Camera const& camera,
	                    ToneMappingModel const& tmm) override;
	void dumpOctreesStates(QString const& dirPath,
	                       QString const& filePathPrefix);
	~CosmologicalSimulation() = default;

	// normalized from 0 to 1
	static float& animationTime();

	static QList<QPair<QString, QWidget*>>
	    getLauncherFields(QWidget* parent, QJsonObject* jsonObj);

  private:
	void init(std::string const& gasOctreePath,
	          std::string const& starsOctreePath,
	          std::string const& darkMatterOctreePath, QColor const& gasColor,
	          QColor const& starsColor, QColor const& darkMatterColor);
	static unsigned int getClosestId(std::map<unsigned int, QString> const& m,
	                                 unsigned index);
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
