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

#ifndef PATHSELECTOR_HPP
#define PATHSELECTOR_HPP

#include <QCompleter>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

class PathSelector : public QWidget
{
	Q_OBJECT
  public:
	PathSelector(QWidget* parent, QString const& caption);

  signals:
	void pathChanged(QString const& path);

  public slots:
	void setPath(QString const& path);

  private:
	QLineEdit* fileEdit;
	QFileSystemModel* dirModel = nullptr;
};

#endif // PATHSELECTOR_HPP
