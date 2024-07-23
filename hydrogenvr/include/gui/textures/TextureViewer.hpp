/*
    Copyright (C) 2023 Florian Cabot <florian.cabot@hotmail.fr>

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

#ifndef TEXTUREVIEWER_HPP
#define TEXTUREVIEWER_HPP

#include <QDialog>
#include <QImage>
#include <QLabel>
#include <QVBoxLayout>

#include "TextureDisplayWindow.hpp"
#include "gl/GLTexture.hpp"

class TextureViewer : public QDialog
{
	Q_OBJECT
  public:
	explicit TextureViewer(GLTexture const& tex, GLShaderProgram&& shader,
	                       QWidget* parent = nullptr);
	virtual void render();
	TextureDisplayWindow& getTextureDisplayWindow();

  protected:
	virtual bool event(QEvent* e) override;

	QVBoxLayout* layout;

	GLShaderProgram shader;
	GLMesh quad;
	TextureDisplayWindow texDispWindow;

	GLTexture const& tex;
};

#endif // TEXTUREVIEWER_HPP
