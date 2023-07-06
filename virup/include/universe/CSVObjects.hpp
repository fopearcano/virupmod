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

#ifndef CSVOBJECTS
#define CSVOBJECTS

#include <QColor>
#include <QFile>
#include <QString>
#include <algorithm>
#include <cmath>

#include "gl/GLHandler.hpp"
#include "graphics/renderers/LabelRenderer.hpp"
#include "physics/Color.hpp"
#include "physics/blackbody.hpp"
#include "universe/UniverseElement.hpp"

class CSVObjects : public UniverseElement
{
  public:
	enum class Designation
	{
		UNKNOWN,
		PROPER,
		BAYER,
		FLAMSTEED
	};

	static QString desigToStr(Designation desig);

	struct StarName
	{
		Designation designation;
		QString name;
	};

	struct Object
	{
		double x;
		double y;
		double z;
		double absmag;
		QColor color;
		std::vector<StarName> names;
	};

	CSVObjects(QJsonObject const& json, bool galaxies = false);
	virtual BBox getBoundingBox() const override;
	virtual void render(Camera const& camera,
	                    ToneMappingModel const& tmm) override;
	void cleanUp();
	virtual ~CSVObjects();

	float colormix = 0.0f;

	float constellationsLabels = 0.f;
	float constellationsAlpha  = 0.f;

	static QList<QPair<QString, QWidget*>>
	    getStarsLauncherFields(QWidget& parent, QJsonObject& jsonObj);
	static QList<QPair<QString, QWidget*>>
	    getGalaxiesLauncherFields(QWidget& parent, QJsonObject& jsonObj);

  private:
	void init(QString const& csvFile, QString const& atlasFile);
	void initWithConstellations(QString const& csvFile,
	                            QString const& constellationsFile);
	static float clamp(float x, float lo, float hi)
	{
		return (x < lo) ? lo : ((x > hi) ? hi : x);
	}
	static QColor colorFromColorIndex(float ci);
	static Object parseLine(QString const& line,
	                        std::map<QString, int> const& columnsNumbers);

	std::vector<Object> objects;
	BBox bbox;

	std::map<QString, unsigned int> indexByName;

	GLShaderProgram shader;
	GLMesh mesh;
	static GLTexture*& starTex();
	static GLTexture*& galTex();

	bool galaxies = false;

	// CONSTELLATIONS
	bool containsConstellations = false;
	GLShaderProgram conShader;
	GLMesh conMesh;

	std::vector<std::pair<Vector3, LabelRenderer*>> conLabels;
};

#endif // CSVOBJECTS
