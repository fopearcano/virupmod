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

#include "gui/ShaderEditor.hpp"

#include "gui/glslSyntaxHighlighter.h"
#include "memory.hpp"

ShaderEditor::ShaderEditor(ShaderProgram& shader, QWidget* parent)
    : QDialog(parent)
{
	setWindowTitle(tr("Shader Editor") + " - " + shader.toStr());

	auto layout = make_qt_unique<QVBoxLayout>(*this);

	auto label = make_qt_unique<QLabel>(*this);
	QString header;
	for(auto const& p : shader.getPipeline())
	{
		header += p.first + '\n';
	}
	header += '\n';
	header += "DEFINES :";
	for(auto const& d : shader.getDefines().keys())
	{
		header += d + '(' + shader.getDefines()[d] + ")\n";
	}
	label->setText(header);
	layout->addWidget(label);

	QStringList files;
	for(auto const& p : shader.getPipeline())
	{
		std::vector<QString> _f;
		GLShaderProgram::getFullPreprocessedSource(
		    p.first + GLShaderProgram::decodeStage(p.second).first,
		    shader.getDefines(), _f);
		for(auto const& file : _f)
		{
			files.append(getAbsoluteDataPath("shaders/" + file));
		}
	}
	files.removeDuplicates();

	auto t = make_qt_unique<QTabWidget>(*this);
	layout->addWidget(t);

	for(auto const& f : files)
	{
		auto w         = make_qt_unique<QWidget>(*this);
		auto tabLayout = make_qt_unique<QVBoxLayout>(*w);

		QFile file(f);
		file.open(QIODevice::ReadOnly | QFile::Text);
		QTextStream in(&file);
		auto text = make_qt_unique<QTextEdit>(*this);
		text->setText(in.readAll().toLocal8Bit());
		file.close();
		tabLayout->addWidget(text);
		(void) make_qt_unique<GlslSyntaxHighlighter>(*text->document());

		auto b = make_qt_unique<QPushButton>(*this, tr("Save"));
		connect(b, &QPushButton::pressed,
		        [text, f]()
		        {
			        QFile::remove(f);
			        QFile file(f);
			        file.open(QIODevice::WriteOnly | QFile::Text);
			        file.write(text->toPlainText().toLatin1());
		        });
		tabLayout->addWidget(b);

		t->addTab(w, f);
	}
	auto b = make_qt_unique<QPushButton>(*this, tr("Reload"));
	connect(b, &QPushButton::pressed, [&shader]() { shader.reload(); });
	layout->addWidget(b);
}
