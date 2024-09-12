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

#include "memory.hpp"

PathSelector::PathSelector(QString const& caption, QWidget* parent)
    : PathSelector(caption, Type::FILE, parent)
{
}

PathSelector::PathSelector(QString const& caption, Type type, QWidget* parent)
    : QWidget(parent)
    , fileEdit(new QLineEdit(this))
{
	QObject::connect(fileEdit, &QLineEdit::textChanged,
	                 [this](QString const& text) { setPath(text); });

	auto* browsePb = make_qt_unique<QPushButton>(*parent);
	browsePb->setText("...");
	QObject::connect(browsePb, &QPushButton::clicked,
	                 [this, caption, type](bool)
	                 {
		                 const QString result(
		                     type == Type::FILE
		                         ? QFileDialog::getOpenFileName(
		                               this, caption, fileEdit->text())
		                         : QFileDialog::getExistingDirectory(
		                               this, caption, fileEdit->text()));
		                 if(result != "")
		                 {
			                 setPath(result);
		                 }
	                 });

	auto* layout = make_qt_unique<QHBoxLayout>(*this);
	layout->addWidget(fileEdit);
	layout->addWidget(browsePb);
}

void PathSelector::setPath(QString const& path)
{
	fileEdit->setText(path);
	auto* dirModel = make_qt_unique<QFileSystemModel>(*fileEdit);
	dirModel->setRootPath(QFileInfo(path).absoluteDir().absolutePath());
	auto* completer = make_qt_unique<QCompleter>(*fileEdit, dirModel);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setCompletionMode(QCompleter::PopupCompletion);
	fileEdit->setCompleter(completer);
	emit pathChanged(path);
}
