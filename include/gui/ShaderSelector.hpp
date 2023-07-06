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

#ifndef SHADERSELECTOR_HPP
#define SHADERSELECTOR_HPP

#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include "ShaderEditor.hpp"
#include "ShaderProgram.hpp"

class ShaderSelector : public QDialog
{
	Q_OBJECT
  public:
	ShaderSelector(QWidget* parent = nullptr);
  public slots:
	virtual void setVisible(bool visible) override;

  private:
	void selectElement(QListWidgetItem* item);
	void setVisibleItems(QString const& match);

	QListWidget listWidget;
};

#endif // SHADERSELECTOR_HPP
