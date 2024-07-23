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

#include "gui/SciDoubleSpinBox.hpp"

SciDoubleSpinBox::SciDoubleSpinBox(QWidget* parent)
    : QDoubleSpinBox(parent)
{
	setRange(-std::numeric_limits<double>::max(),
	         std::numeric_limits<double>::max());
	setDecimals(323);

	connect(this,
	        static_cast<void (QDoubleSpinBox::*)(double)>(
	            &QDoubleSpinBox::valueChanged),
	        [this](double val)
	        {
		        int tenPow(floor(log10(abs(val))));
		        setSingleStep(pow(10, tenPow - 1));
	        });
}

double SciDoubleSpinBox::valueFromText(QString const& text) const
{
	return text.toDouble();
}

QString SciDoubleSpinBox::textFromValue(double value) const
{
	return QString::number(value, 'E', 6);
}

QValidator::State SciDoubleSpinBox::validate(QString& text, int& /*pos*/) const
{
	// Try to convert the string to double
	bool ok;
	text.toDouble(&ok);
	// See if it's a valid Double
	return ok ? QValidator::Acceptable : QValidator::Invalid;
}
