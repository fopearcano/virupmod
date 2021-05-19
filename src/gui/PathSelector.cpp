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

#include "gui/PathSelector.hpp"

PathSelector::PathSelector(QWidget* parent, QString const& caption)
    : QWidget(parent)
    , fileEdit(new QLineEdit(this))
{
	QObject::connect(fileEdit, &QLineEdit::textChanged,
	                 [this](QString const& text) { setPath(text); });

	auto browsePb = new QPushButton(parent);
	browsePb->setText("...");
	QObject::connect(browsePb, &QPushButton::clicked, [this, caption](bool) {
		QString result(
		    QFileDialog::getOpenFileName(this, caption, fileEdit->text()));
		if(result != "")
		{
			setPath(result);
		}
	});

	auto layout = new QHBoxLayout(this);
	layout->addWidget(fileEdit);
	layout->addWidget(browsePb);
}

void PathSelector::setPath(QString const& path)
{
	fileEdit->setText(path);
	dirModel = new QFileSystemModel;
	dirModel->setRootPath(QFileInfo(path).absoluteDir().absolutePath());
	auto completer = new QCompleter(dirModel);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setCompletionMode(QCompleter::PopupCompletion);
	fileEdit->setCompleter(completer);
	emit pathChanged(path);
}
