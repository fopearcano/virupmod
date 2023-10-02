/*
    Copyright (C) 2022 Florian Cabot <florian.cabot@hotmail.fr>

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

#ifndef TEXTURESELECTOR_HPP
#define TEXTURESELECTOR_HPP

#include <QDialog>
#include <QListWidget>

#include "TextureViewer.hpp"
#include "gl/GLTexture.hpp"

class TextureSelector : public QDialog
{
	Q_OBJECT
  public:
	TextureSelector(QWidget* parent = nullptr);
	TextureViewer* getViewer() { return viewer; };
  public slots:
	virtual void setVisible(bool visible) override;

  private:
	void selectElement(QListWidgetItem* item);
	void setVisibleItems(QString const& match);

	QListWidget listWidget;
	TextureViewer* viewer = nullptr;
};

#endif // TEXTURESELECTOR_HPP
