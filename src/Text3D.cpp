/*
    Copyright (C) 2019 Florian Cabot <florian.cabot@hotmail.fr>

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

#include "Text3D.hpp"

#include <QOpenGLPaintDevice>

Text3D::Text3D(unsigned int width, unsigned int height)
    : Text3D(width, height, GLShaderProgram("billboard"))
{
}

Text3D::Text3D(unsigned int width, unsigned int height,
               GLShaderProgram&& shader)
    : shader(std::move(shader))
    , originalSize(width, height)
{
	Primitives::setAsQuad(quad, this->shader);
	if(width > height)
	{
		aspectratio.scale(1.f, static_cast<float>(height) / width);
	}
	else
	{
		aspectratio.scale(static_cast<float>(width) / height, 1.f);
	}
	updateTex();
}

void Text3D::setText(QString const& text)
{
	this->text = text;
	updateTex();
}

void Text3D::setColor(QColor const& color)
{
	this->color = color;
	updateTex();
}

void Text3D::setAlpha(float alpha)
{
	this->alpha = alpha;
	shader.setUniform("alpha", alpha);
}

void Text3D::setFont(QFont const& font)
{
	this->font = font;
	updateTex();
}

void Text3D::setBackgroundColor(QColor const& backgroundColor)
{
	this->backgroundColor = backgroundColor;
	updateTex();
}

void Text3D::setRectangle(QRect const& rectangle)
{
	this->rectangle = rectangle;
	updateTex();
}

void Text3D::setFlags(int flags)
{
	this->flags = flags;
	updateTex();
}

void Text3D::setSuperSampling(float superSampling)
{
	this->superSampling = superSampling;
	updateTex();
}

void Text3D::render(GLHandler::GeometricSpace geometricSpace)
{
	if(alpha < 0.01)
	{
		return;
	}

	GLBlendSet glBlend(GLBlendSet::BlendState{});
	GLHandler::setUpRender(shader, model * aspectratio, geometricSpace);
	GLHandler::useTextures({&fbo->getColorAttachmentTexture()});
	quad.render(PrimitiveType::TRIANGLE_STRIP);
}

void Text3D::updateTex()
{
	if(fbo == nullptr || superSampling * originalSize != fbo->getSize())
	{
		fbo = std::make_unique<GLFramebufferObject>(
		    GLTexture::Tex2DProperties(superSampling * originalSize.width(),
		                               superSampling * originalSize.height()));
	}

	bool sizeInPixels(true);
	int fontSize(font.pixelSize());
	if(fontSize == -1)
	{
		sizeInPixels = false;
		fontSize     = font.pointSize();
	}

	if(sizeInPixels)
	{
		font.setPixelSize(static_cast<int>(superSampling * fontSize));
	}
	else
	{
		font.setPointSize(static_cast<int>(superSampling * fontSize));
	}

	QRect adjustedRect(static_cast<int>(superSampling * rectangle.x()),
	                   static_cast<int>(superSampling * rectangle.y()),
	                   static_cast<int>(superSampling * rectangle.width()),
	                   static_cast<int>(superSampling * rectangle.height()));

	paintText(*fbo, text, color, font, backgroundColor, adjustedRect, flags);

	if(sizeInPixels)
	{
		font.setPixelSize(fontSize);
	}
	else
	{
		font.setPointSize(fontSize);
	}
}

QRect Text3D::paintText(GLFramebufferObject& fbo, QString const& text,
                        QColor const& color, QFont const& font,
                        QColor const& backgroundColor, QRect const& rectangle,
                        int flags)
{
	fbo.bind();
	GLHandler::setClearColor(backgroundColor);
	GLHandler::glf().glClear(GL_COLOR_BUFFER_BIT);
	GLHandler::setClearColor(Qt::black);

	QOpenGLPaintDevice d(fbo.getSize());
	QPainter painter(&d);

	QRect boundingRect;

	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::TextAntialiasing);

	painter.setFont(font);
	painter.setPen(color);
	if(rectangle.isNull())
	{
		painter.drawText(0, 0, fbo.getSize().width(), fbo.getSize().height(),
		                 flags, text, &boundingRect);
	}
	else
	{
		painter.drawText(rectangle, flags, text, &boundingRect);
	}

	return boundingRect;
}
