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

class SceneUI
{
  public:
	SceneUI()                     = default;
	SceneUI(SceneUI const& other) = default;
	SceneUI(SceneUI&& other)      = default;
	SceneUI(std::map<QString, float> visibilities);
	SceneUI(QVariantMap const& visibilities);
	SceneUI& operator=(SceneUI const& ui) = default;

	float getVisibility(QString const& name) const;
	void setVisibility(QString const& name, float vis);
	static SceneUI getCurrentState(Universe const& universe);
	void setAsUniverseState(Universe& universe) const;

	static SceneUI interpolate(SceneUI const& ui0, SceneUI const& ui1, float t);

	QString getPythonRepresentation() const;

  private:
	std::map<QString, float> visibilities = {};

	static std::map<QString, float> fromQt(QVariantMap const& map);
};

/* PYTHONQT */

Q_DECLARE_METATYPE(SceneUI)

class SceneUIWrapper : public PythonQtWrapper
{
	Q_OBJECT
  public:
	virtual const char* wrappedClassName() const override { return "SceneUI"; }
	virtual const char* wrappedClassPackage() const override { return "virup"; }

  public Q_SLOTS:
	SceneUI* new_SceneUI() { return new SceneUI; }
	SceneUI* new_SceneUI(SceneUI const& ui) { return new SceneUI(ui); }
	SceneUI* new_SceneUI(QVariantMap const& map) { return new SceneUI(map); }

	void delete_SceneUI(SceneUI* f) { delete f; }

	// access methods
	float getVisibility(SceneUI* u, QString const& name)
	{
		return u->getVisibility(name);
	};
	void setVisibility(SceneUI* u, QString const& name, float vis)
	{
		u->setVisibility(name, vis);
	};

	SceneUI static_SceneUI_getCurrentState(Universe const& universe)
	{
		return SceneUI::getCurrentState(universe);
	};
	void setAsUniverseState(SceneUI* ui, Universe& universe) const
	{
		ui->setAsUniverseState(universe);
	};
	SceneUI static_SceneUI_interpolate(SceneUI const& ui0, SceneUI const& ui1,
	                                   float t)
	{
		return SceneUI::interpolate(ui0, ui1, t);
	}
};

#endif // SCENEUI_HPP
