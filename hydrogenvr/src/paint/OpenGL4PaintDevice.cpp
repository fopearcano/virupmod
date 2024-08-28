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

#include "paint/OpenGL4PaintDevice.hpp"

#include <QtDebug>

#include "gl/GLHandler.hpp"

bool OpenGL4PaintEngine::begin(QPaintDevice* /*pdev*/)
{
	return true;
}

void OpenGL4PaintEngine::drawImage(QRectF const& rectangle, QImage const& image,
                                   QRectF const& sr,
                                   Qt::ImageConversionFlags /*flags*/)
{
	// Make sure the current FBO binding state doesn't get messed up
	GLFramebufferObject::pushCurrentOnBindStack();

	GLTexture tex(image);
	// currentGLFBO() = std::make_unique<GLFramebufferObject>(std::move(tex));
	GLFramebufferObject fbo(std::move(tex));

	GLFramebufferObject::applyCurrentFromBindStack();

	int curHeight(GLFramebufferObject::getCurrentSize()[1]);
	fbo.blitColorBufferToCurrent(
	    sr.x(), sr.y(), sr.x() + sr.width(), sr.y() + sr.height(),
	    rectangle.x(), curHeight - rectangle.y() - rectangle.width(),
	    rectangle.x() + rectangle.width(), curHeight - rectangle.y());

	GLFramebufferObject::popCurrentFromBindStack();
}

void OpenGL4PaintEngine::drawPixmap(QRectF const& r, QPixmap const& pm,
                                    QRectF const& sr)
{
	qWarning() << "Unimplemented" << r << pm << sr;
}

void OpenGL4PaintEngine::drawPolygon(QPointF const* points, int pointCount,
                                     QPaintEngine::PolygonDrawMode mode)
{
	(void) mode;
	if(pointCount <= 0)
	{
		return;
	}

	int width(paintDevice()->width()), height(paintDevice()->height());
	std::vector<float> vertices;
	for(int i(0); i < pointCount; ++i)
	{
		vertices.push_back(2 * points[i].x() / width - 1);
		vertices.push_back(-2 * points[i].y() / height + 1);
	}
	if(mode != QPaintEngine::PolylineMode)
	{
		vertices.push_back(2 * points[0].x() / width - 1);
		vertices.push_back(-2 * points[0].y() / height + 1);
	}

	defaultLineShader.setUniform("color", pen.color());
	GLMesh mesh;
	mesh.setVertexShaderMapping(defaultLineShader, {{"position", 2}});
	mesh.setVertices(vertices);
	GLHandler::setUpRender(defaultLineShader, {},
	                       GLHandler::GeometricSpace::CLIP);
	mesh.render(PrimitiveType::LINE_STRIP);
}

void OpenGL4PaintEngine::drawTextItem(QPointF const& p,
                                      QTextItem const& textItem)
{
	// Use Qt's OpenGL text rendering logic
	// Dirty but doesn't reinvent a large wheel
	QOpenGLPaintDevice d(this->paintDevice()->width(),
	                     this->paintDevice()->height());
	QPainter painter(&d);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::TextAntialiasing);

	painter.setFont(textItem.font());
	painter.setPen(pen);
	painter.beginNativePainting();
	painter.drawText(p.x(), p.y(), textItem.text());
}

bool OpenGL4PaintEngine::end()
{
	return true;
}

QPaintEngine::Type OpenGL4PaintEngine::type() const
{
	return QPaintEngine::OpenGL;
}

void OpenGL4PaintEngine::updateState(QPaintEngineState const& state)
{
	QPaintEngine::DirtyFlags dirty(state.state());
	if((dirty & QPaintEngine::DirtyPen) != 0u)
	{
		pen = state.pen();
		// dirty &= !QPaintEngine::DirtyPen;
	}
	// qWarning() << "Unimplemented" << dirty;
}

QPaintEngine* OpenGL4PaintDevice::paintEngine() const
{
	// Return an instance of your custom paint engine
	return &engine;
}
