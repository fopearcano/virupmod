/*
    Copyright (C) 2022 Florian Cabot <florian.cabot@hotmail.fr>

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

#include "gui/textures/TextureSelector.hpp"

#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "gui/textures/TEX2DViewer.hpp"
#include "gui/textures/TEXCUBEMAPViewer.hpp"
#include "memory.hpp"

TextureSelector::TextureSelector(QWidget* parent)
    : QDialog(parent)
{
	setFixedSize(450, 600);
	setWindowTitle(tr("Texture Selector"));

	auto layout = make_qt_unique<QVBoxLayout>(*this);

	auto w            = make_qt_unique<QWidget>(*this);
	auto layoutSearch = make_qt_unique<QHBoxLayout>(*w);

	auto searchLabel = make_qt_unique<QLabel>(*w);
	searchLabel->setText(tr("Search :"));
	auto searchBar = make_qt_unique<QLineEdit>(*w);
	connect(searchBar, &QLineEdit::textChanged, this,
	        &TextureSelector::setVisibleItems);
	layoutSearch->addWidget(searchLabel);
	layoutSearch->addWidget(searchBar);
	layout->addWidget(w);

	auto b = make_qt_unique<QPushButton>(*this);
	b->setText(tr("Refresh"));
	connect(b, &QPushButton::pressed, [this]() { setVisible(true); });
	layout->addWidget(b);

	connect(&listWidget, &QListWidget::itemActivated, this,
	        &TextureSelector::selectElement);
	layout->addWidget(&listWidget);

	b = make_qt_unique<QPushButton>(*this);
	b->setText(tr("View"));
	connect(b, &QPushButton::pressed,
	        [this]() { selectElement(listWidget.currentItem()); });
	layout->addWidget(b);
}

void TextureSelector::setVisible(bool visible)
{
	if(visible)
	{
		listWidget.clear();
		for(auto texture : GLTexture::getAllTextures())
		{
			QString label;
			QString glID(QString::number(texture->getGLTexture()));
			label = glID;
			if(!texture->getName().isEmpty())
			{
				label += " - " + texture->getName();
			}
			label += " - " + texture->getTypeStr();
			auto item = std::make_unique<QListWidgetItem>(label);
			item->setData(Qt::UserRole, glID);
			listWidget.addItem(item.release());
		}
	}
	QDialog::setVisible(visible);
}

void TextureSelector::selectElement(QListWidgetItem* item)
{
	auto glID    = item->data(Qt::UserRole);
	GLTexture* t = nullptr;
	for(auto texture : GLTexture::getAllTextures())
	{
		if(QString::number(texture->getGLTexture()) == glID)
		{
			t = texture;
		}
	}

	if(t == nullptr)
	{
		return;
	}

	switch(t->getType())
	{
		case GLTexture::Type::TEX2D:
		{
			viewer = make_qt_unique<TEX2DViewer>(*this, *t);
			viewer->show();
			connect(viewer, &QDialog::finished, [this]() { viewer = nullptr; });
		}
		break;
		case GLTexture::Type::TEX1D:
		case GLTexture::Type::TEX3D:
		case GLTexture::Type::TEXMULTISAMPLE:
		case GLTexture::Type::TEXCUBEMAP:
		{
			viewer = make_qt_unique<TEXCUBEMAPViewer>(*this, *t);
			viewer->show();
			connect(viewer, &QDialog::finished, [this]() { viewer = nullptr; });
		}
		break;
		default:
			QMessageBox::warning(
			    this, tr("Unsupported texture type"),
			    tr("This texture type doesn't have a viewer implemented yet."));
	}
}

void TextureSelector::setVisibleItems(QString const& match)
{
	for(auto item : listWidget.findItems("", Qt::MatchContains))
	{
		if(match == ""
		   || item->text().contains(QString(match), Qt::CaseInsensitive))
		{
			item->setHidden(false);
		}
		else
		{
			item->setHidden(true);
		}
	}
}
