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

#include "gui/FontSelector.hpp"

#include <QFontDialog>

FontSelector::FontSelector(QString const& caption, QWidget* parent)
    : QPushButton(parent)
{
	setObjectName("mainbutton");
	connect(this, &QPushButton::clicked, this,
	        [this, caption](bool)
	        {
		        bool ok = false;
		        const QFont result(
		            QFontDialog::getFont(&ok, currentFont, this, caption));
		        if(!ok)
		        {
			        return;
		        }
		        setFont(result);
	        });
}

void FontSelector::setFont(QFont const& font)
{
	QPushButton::setFont(font);
	setText(font.family());
	currentFont = font;
	emit fontChanged(font);
}
