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

#ifndef SCENESELECTOR_HPP
#define SCENESELECTOR_HPP

#include <QMouseEvent>

#include "Dialog3D.hpp"
#include "scenes/Animator.hpp"

class SceneSelector : public Dialog3D
{
  public:
	SceneSelector(Animator& animator);
	bool voiceOverIsEnglish() const { return english; };
	void update();

  private:
	Animator& animator;

	bool english                      = true;
	std::vector<QPushButton*> buttons = {};
	QPushButton* transitionsButton    = nullptr;
	QSlider slider;
	bool animateSlider = true;
};

#endif // SCENESELECTOR_HPP
