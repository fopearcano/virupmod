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

#ifndef SCIDOUBLESPINBOX_HPP
#define SCIDOUBLESPINBOX_HPP

#include <QDoubleSpinBox>
#include <cmath>

// QDoubleSpinBox but with scientific notation

// https://stackoverflow.com/questions/54295639/how-to-set-qdoublespinbox-to-scientific-notation
class SciDoubleSpinBox : public QDoubleSpinBox
{
	Q_OBJECT
  public:
	explicit SciDoubleSpinBox(QWidget* parent = 0);
	// Change the way we read the user input
	double valueFromText(QString const& text) const override;
	// Change the way we show the internal number
	QString textFromValue(double value) const override;
	// Change the way we validate user input (if validate => valueFromText)
	QValidator::State validate(QString& text, int& pos) const override;
};

#endif // SCIDOUBLESPINBOX_HPP
