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

#include "paint/AdvancedPainter.hpp"

#include <QtDebug>
#include <cfloat>

#include "gl/GLHandler.hpp"

QRectF
    AdvancedPainter::drawFunction(QRectF const& target, float xBegin,
                                  float xEnd,
                                  std::function<float(float)> const& function)
{
	std::vector<QPointF> dataPoints;
	float miny = FLT_MAX, maxy = FLT_MIN;
	const unsigned int N(target.width());
	// construct polygon in data space
	for(unsigned int i(0); i < N; ++i)
	{
		const float x = xBegin + i * (xEnd - xBegin) / (N - 1);
		const float y = function(x);
		miny          = miny < y ? miny : y;
		maxy          = maxy > y ? maxy : y;
		dataPoints.emplace_back(x, y);
	}

	QPolygonF polygon;
	// transform polygon in device space
	// => map xBegin, xEnd -> target.x(), target.x() + target.width()
	// => map miny, maxy -> target.y() + target.height(), target.y()
	for(auto& point : dataPoints)
	{
		// normalize
		point = QPointF{(point.x() - xBegin) / (xEnd - xBegin),
		                (point.y() - miny) / (maxy - miny)};
		// unnormalize
		point = QPointF{target.x() + point.x() * target.width(),
		                target.y() + target.height()
		                    + point.y() * -target.height()};
		polygon << point;
	}

	drawPolyline(polygon);

	return {xBegin, miny, xEnd - xBegin, maxy - miny};
}

QRectF
    AdvancedPainter::drawFunction(QRectF const& target, float xBegin,
                                  float xEnd, float yBegin,
                                  std::function<float(float)> const& function)
{
	std::vector<QPointF> dataPoints;
	float maxy = FLT_MIN;
	const unsigned int N(target.width());
	// construct polygon in data space
	for(unsigned int i(0); i < N; ++i)
	{
		const float x = xBegin + i * (xEnd - xBegin) / (N - 1);
		float y       = function(x);
		y             = y < yBegin ? yBegin : y;
		maxy          = maxy > y ? maxy : y;
		dataPoints.emplace_back(x, y);
	}

	QPolygonF polygon;
	// transform polygon in device space
	// => map xBegin, xEnd -> target.x(), target.x() + target.width()
	// => map miny, maxy -> target.y() + target.height(), target.y()
	for(auto& point : dataPoints)
	{
		// normalize
		point = QPointF{(point.x() - xBegin) / (xEnd - xBegin),
		                (point.y() - yBegin) / (maxy - yBegin)};
		// unnormalize
		point = QPointF{target.x() + point.x() * target.width(),
		                target.y() + target.height()
		                    + point.y() * -target.height()};
		polygon << point;
	}

	drawPolyline(polygon);

	return {xBegin, yBegin, xEnd - xBegin, maxy - yBegin};
}

QRectF
    AdvancedPainter::drawFunction(QRectF const& target, float xBegin,
                                  float xEnd, float yBegin, float yEnd,
                                  std::function<float(float)> const& function)
{
	QPolygonF polygon;
	const unsigned int N(target.width());
	// construct polygon in data space
	for(unsigned int i(0); i < N; ++i)
	{
		const float x(xBegin + i * (xEnd - xBegin) / (N - 1));
		float y = function(x);
		y       = y < yBegin ? yBegin : y;
		y       = y > yEnd ? yEnd : y;
		QPointF point(x, y);
		// transform polygon in device space
		// => map xBegin, xEnd -> target.x(), target.x() + target.width()
		// => map miny, maxy -> target.y() + target.height(), target.y()
		// normalize
		point = QPointF{(point.x() - xBegin) / (xEnd - xBegin),
		                (point.y() - yBegin) / (yEnd - yBegin)};
		// unnormalize
		point = QPointF{target.x() + point.x() * target.width(),
		                target.y() + target.height()
		                    + point.y() * -target.height()};
		polygon << point;
	}
	drawPolyline(polygon);

	return {xBegin, yBegin, xEnd - xBegin, yEnd - yBegin};
}

QRectF AdvancedPainter::drawBarPlot(QRectF const& target,
                                    std::vector<float> const& values)
{
	const auto pair = std::minmax_element(values.begin(), values.end());
	auto minVal(*pair.first), maxVal(*pair.second);
	return drawBarPlot(target, minVal, maxVal, values);
}

QRectF AdvancedPainter::drawBarPlot(QRectF const& target, float yBegin,
                                    float yEnd,
                                    std::vector<float> const& values)
{
	QVector<QRectF> rects;
	for(unsigned int i(0); i < values.size(); ++i)
	{
		const QPointF tl{static_cast<float>(i) / values.size(),
		                 (values.at(i) - yBegin) / (yEnd - yBegin)};
		const QPointF br{static_cast<float>(i + 1) / values.size(), 0.f};
		rects.push_back({transform(target, tl), transform(target, br)});
	}
	QPainter::drawRects(rects);

	return {0.f, yBegin, 1.f, yEnd - yBegin};
}

void AdvancedPainter::drawHistogram(QRectF const& target,
                                    GLBuffer const& histData)
{
	const QSize fboSize(device()->width(), device()->height());
	const GLShaderProgram histShader("histogram");
	GLMesh quad;
	quad.setVertexShaderMapping(histShader, {{"position", 2}});
	quad.setVertices({-1.f, -1.f, 1.f, -1.f, -1.f, 1.f, 1.f, 1.f});

	const float relX(target.x() / fboSize.width()),
	    relY(1.0 - (target.y() + target.height()) / fboSize.height()),
	    relWidth(target.width() / fboSize.width()),
	    relHeight(target.height() / fboSize.height());
	QMatrix4x4 model;
	model.scale(relWidth, relHeight);
	model.translate((2.0 * relX - 1.0) / relWidth + 1.0,
	                (2.0 * relY - 1.0) / relHeight + 1.0);

	histData.bindBase(0);
	histShader.setUniform("color", pen().color());
	histShader.setUniform("bufferSize",
	                      static_cast<int>(histData.getSize() / sizeof(int)));

	const GLBlendSet glBlend(GLBlendSet::BlendState{});
	GLHandler::setUpRender(histShader, model, GLHandler::GeometricSpace::CLIP);
	quad.render(PrimitiveType::TRIANGLE_STRIP);
}

void AdvancedPainter::fillRect(QRectF const& target)
{
	const GLStateSet glState({{GL_SCISSOR_TEST, true}});
	GLHandler::glf().glScissor(
	    target.x(), device()->height() - target.height() - target.y(),
	    target.width(), target.height());
	GLHandler::setClearColor(pen().color());
	GLHandler::glf().glClear(GL_COLOR_BUFFER_BIT);
	GLHandler::setClearColor(Qt::black);
}

QPointF AdvancedPainter::transform(QRectF const& absoluteRef,
                                   QPointF const& normalized, bool flipped)
{
	auto nx(normalized.x()),
	    ny(flipped ? 1.f - normalized.y() : normalized.y());
	return {absoluteRef.x() + nx * absoluteRef.width(),
	        absoluteRef.y() + ny * absoluteRef.height()};
}

QPointF AdvancedPainter::invTransform(QRectF const& absoluteRef,
                                      QPointF const& absoluteCoord,
                                      bool flipped)
{
	const float x((absoluteCoord.x() - absoluteRef.x()) / absoluteRef.width());
	float y((absoluteCoord.y() - absoluteRef.y()) / absoluteRef.height());
	if(flipped)
	{
		y = 1.f - y;
	}
	return {x, y};
}
