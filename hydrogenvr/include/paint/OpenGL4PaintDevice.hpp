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

#ifndef OPENGL4PAINTDEVICE_HPP
#define OPENGL4PAINTDEVICE_HPP

#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtOpenGL/QOpenGLPaintDevice>
#else
#include <QOpenGLPaintDevice>
#endif
#include <QPaintEngine>

#include "gl/GLShaderProgram.hpp"

class OpenGL4PaintEngine : public QPaintEngine
{
  public:
	OpenGL4PaintEngine()
	    : QPaintEngine()
	    , defaultLineShader("defaultline"){};
	virtual bool begin(QPaintDevice* pdev) override;
	virtual void drawImage(QRectF const& rectangle, QImage const& image,
	                       QRectF const& sr, Qt::ImageConversionFlags flags
	                       /*= Qt::AutoColor*/) override;
	virtual void drawPixmap(QRectF const& r, QPixmap const& pm,
	                        QRectF const& sr) override;
	virtual void drawPolygon(QPointF const* points, int pointCount,
	                         QPaintEngine::PolygonDrawMode mode) override;
	virtual void drawTextItem(QPointF const& p,
	                          QTextItem const& textItem) override;
	virtual bool end() override;
	virtual QPaintEngine::Type type() const override;
	virtual void updateState(QPaintEngineState const& state) override;

  private:
	QPen pen;

	GLShaderProgram defaultLineShader;
};

class OpenGL4PaintDevice : public QOpenGLPaintDevice
{
  public:
	OpenGL4PaintDevice(QSize const& size)
	    : QOpenGLPaintDevice(size){};
	virtual QPaintEngine* paintEngine() const override;

  private:
	mutable OpenGL4PaintEngine engine;
};

#endif // OPENGL4PAINTDEVICE_HPP
