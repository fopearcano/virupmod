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

#ifndef ADVANCEDPAINTER_HPP
#define ADVANCEDPAINTER_HPP

#include <QPainter>
#include <functional>

class GLBuffer;

class AdvancedPainter : public QPainter
{
  public:
	AdvancedPainter(QPaintDevice* device)
	    : QPainter(device){};
	// returned QRectF is the data rect (minx, maxx, miny, maxy)
	QRectF drawFunction(QRectF const& target, float xBegin, float xEnd,
	                    std::function<float(float)> const& function);
	QRectF drawFunction(QRectF const& target, float xBegin, float xEnd,
	                    float yBegin,
	                    std::function<float(float)> const& function);
	QRectF drawFunction(QRectF const& target, float xBegin, float xEnd,
	                    float yBegin, float yEnd,
	                    std::function<float(float)> const& function);
	QRectF drawBarPlot(QRectF const& target, std::vector<float> const& values);
	QRectF drawBarPlot(QRectF const& target, float yBegin, float yEnd,
	                   std::vector<float> const& values);
	void drawHistogram(QRectF const& target, GLBuffer const& histData);
	void fillRect(QRectF const& target);

  private:
	// transforms normalized coordinates (0->1) to absolute coordinate inside
	// absoluteRef
	static QPointF transform(QRectF const& absoluteRef,
	                         QPointF const& normalized, bool flipped = true);

	// transforms absolute coordinate inside
	// absoluteRef to normalized coordinates (0->1)
	static QPointF invTransform(QRectF const& absoluteRef,
	                            QPointF const& absoluteCoord,
	                            bool flipped = true);
};

#endif // ADVANCEDPAINTER_HPP
