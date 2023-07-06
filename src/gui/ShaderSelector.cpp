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

#include "gui/ShaderSelector.hpp"

#include "memory.hpp"

ShaderSelector::ShaderSelector(QWidget* parent)
    : QDialog(parent)
{
	setFixedSize(450, 600);
	setWindowTitle(tr("Shader Selector"));

	auto layout = make_qt_unique<QVBoxLayout>(*this);

	auto w            = make_qt_unique<QWidget>(*this);
	auto layoutSearch = make_qt_unique<QHBoxLayout>(*w);

	auto searchLabel = make_qt_unique<QLabel>(*w);
	searchLabel->setText(tr("Search :"));
	auto searchBar = make_qt_unique<QLineEdit>(*w);
	connect(searchBar, &QLineEdit::textChanged, this,
	        &ShaderSelector::setVisibleItems);
	layoutSearch->addWidget(searchLabel);
	layoutSearch->addWidget(searchBar);
	layout->addWidget(w);

	auto b = make_qt_unique<QPushButton>(*this);
	b->setText(tr("Refresh"));
	connect(b, &QPushButton::pressed, [this]() { setVisible(true); });
	layout->addWidget(b);

	connect(&listWidget, &QListWidget::itemActivated, this,
	        &ShaderSelector::selectElement);
	layout->addWidget(&listWidget);

	b = make_qt_unique<QPushButton>(*this);
	b->setText(tr("Edit"));
	connect(b, &QPushButton::pressed,
	        [this]() { selectElement(listWidget.currentItem()); });
	layout->addWidget(b);
}

void ShaderSelector::setVisible(bool visible)
{
	if(visible)
	{
		listWidget.clear();
		for(auto shader : ShaderProgram::getAllShaderPrograms())
		{
			QString glID(shader->toStr());
			QString pipelineStr;
			for(auto const& p : shader->getPipeline())
			{
				pipelineStr += p.first.split('/').last() + '/';
			}
			pipelineStr.chop(1);
			QString defines;
			for(auto const& d : shader->getDefines().keys())
			{
				defines += d + '(' + shader->getDefines()[d] + "), ";
			}
			auto item = new QListWidgetItem(glID + " - " + pipelineStr + " - "
			                                + defines);
			item->setData(Qt::UserRole, glID);
			listWidget.addItem(item);
		}
	}
	QDialog::setVisible(visible);
}

void ShaderSelector::selectElement(QListWidgetItem* item)
{
	auto glID         = item->data(Qt::UserRole);
	ShaderProgram* sp = nullptr;
	for(auto shader : ShaderProgram::getAllShaderPrograms())
	{
		if(shader->toStr() == glID)
		{
			sp = shader;
		}
	}

	if(sp == nullptr)
	{
		return;
	}

	auto editor = make_qt_unique<ShaderEditor>(*this, *sp);
	editor->show();
}

void ShaderSelector::setVisibleItems(QString const& match)
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
