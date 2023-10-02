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

#include "gui/textures/TextureViewer.hpp"

TextureViewer::TextureViewer(GLTexture const& tex, GLShaderProgram&& shader,
                             QWidget* parent)
    : QDialog(parent)
    , shader(std::move(shader))
    , tex(tex)
{
	QString title("Texture " + QString::number(tex.getGLTexture()));
	if(!tex.getName().isEmpty())
	{
		title += " - " + tex.getName();
	}
	setWindowTitle(title);

	auto textLabel = make_qt_unique<QLabel>(*this, "Your Image:");
	layout         = make_qt_unique<QVBoxLayout>(*this);

	textLabel->setAlignment(Qt::AlignCenter);
	layout->addWidget(textLabel);

	auto w = QWidget::createWindowContainer(&texDispWindow, this);
	w->setMinimumSize(100, 100);
	w->setMaximumSize(2000, 2000);
	w->setFocusPolicy(Qt::TabFocus);
	layout->addWidget(w);

	quad.setVertexShaderMapping(this->shader, {{"position", 2}});
	quad.setVertices({-1.f, -1.f, 1.f, -1.f, -1.f, 1.f, 1.f, 1.f});
}

void TextureViewer::render()
{
	GLHandler::glf().glViewport(0, 0, texDispWindow.geometry().width(),
	                            texDispWindow.geometry().height());
	GLHandler::useTextures({&tex});
	shader.use();
	GLHandler::setBackfaceCulling(false);
	quad.render(PrimitiveType::TRIANGLE_STRIP);
	GLHandler::setBackfaceCulling(true);
}

TextureDisplayWindow& TextureViewer::getTextureDisplayWindow()
{
	return texDispWindow;
}

bool TextureViewer::event(QEvent* e)
{
	if(e->type() == QEvent::Type::Close)
	{
		emit QDialog::finished(0);
	}
	return QDialog::event(e);
}
