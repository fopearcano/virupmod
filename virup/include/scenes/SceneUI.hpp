/*
    Copyright (C) 2021 Florian Cabot <florian.cabot@hotmail.fr>

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

#ifndef SCENEUI_HPP
#define SCENEUI_HPP

#include "universe/Universe.hpp"

/*! \ingroup pywrap
 *
 * To use this class in a Python script :
 * \code{.py}
 * from PythonQt.virup import SceneUI
 * \endcode
 */
class SceneUI
{
  public:
	SceneUI()                     = default;
	SceneUI(SceneUI const& other) = default;
	SceneUI(SceneUI&& other)      = default;
	explicit SceneUI(std::map<QString, float> visibilities);
	explicit SceneUI(QVariantMap const& visibilities);
	SceneUI& operator=(SceneUI const& ui) = default;
	SceneUI& operator=(SceneUI&& ui)      = default;
	~SceneUI()                            = default;

	float getVisibility(QString const& name) const;
	void setVisibility(QString const& name, float vis);
	static SceneUI getCurrentState(Universe const& universe);
	void setAsUniverseState(Universe& universe) const;

	static SceneUI interpolate(SceneUI const& ui0, SceneUI const& ui1, float t);

	QString getPythonRepresentation() const;

  private:
	std::map<QString, float> visibilities;

	static std::map<QString, float> fromQt(QVariantMap const& map);
};

/* PYTHONQT */

Q_DECLARE_METATYPE(SceneUI)

class SceneUIWrapper : public PythonQtWrapper
{
	Q_OBJECT
  public:
	const char* wrappedClassName() const override { return "SceneUI"; }
	const char* wrappedClassPackage() const override { return "virup"; }

  public Q_SLOTS:
	static SceneUI* new_SceneUI() { return new SceneUI; }
	static SceneUI* new_SceneUI(SceneUI const& ui) { return new SceneUI(ui); }
	static SceneUI* new_SceneUI(QVariantMap const& map)
	{
		return new SceneUI(map);
	}

	static void delete_SceneUI(SceneUI* f) { delete f; }

	// access methods
	static float getVisibility(SceneUI* u, QString const& name)
	{
		return u->getVisibility(name);
	};
	static void setVisibility(SceneUI* u, QString const& name, float vis)
	{
		u->setVisibility(name, vis);
	};

	static SceneUI static_SceneUI_getCurrentState(Universe const& universe)
	{
		return SceneUI::getCurrentState(universe);
	};
	static void setAsUniverseState(SceneUI* ui, Universe& universe)
	{
		ui->setAsUniverseState(universe);
	};
	static SceneUI static_SceneUI_interpolate(SceneUI const& ui0,
	                                          SceneUI const& ui1, float t)
	{
		return SceneUI::interpolate(ui0, ui1, t);
	}
};

#endif // SCENEUI_HPP
