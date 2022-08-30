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

ShaderEditor::ShaderEditor(ShaderProgram& shader, QWidget* parent)
    : QDialog(parent)
{
	setWindowTitle(tr("Shader Editor") + " - " + shader.toStr());

	auto layout = new QVBoxLayout(this);

	auto label = new QLabel(this);
	QString header(shader.getVertexShaderPath() + '\n'
	               + shader.getFragmentShaderPath() + "\n\n");
	header += "DEFINES :";
	for(auto const& d : shader.getDefines().keys())
	{
		header += d + '(' + shader.getDefines()[d] + ")\n";
	}
	label->setText(header);
	layout->addWidget(label);

	QStringList files;
	std::vector<QString> _f;
	GLShaderProgram::getFullPreprocessedSource(
	    shader.getVertexShaderPath()
	        + GLShaderProgram::decodeStage(GLShaderProgram::Stage::VERTEX)
	              .first,
	    shader.getDefines(), _f);
	for(auto const& file : _f)
	{
		files.append(getAbsoluteDataPath("shaders/" + file));
	}
	_f.clear();
	GLShaderProgram::getFullPreprocessedSource(
	    shader.getFragmentShaderPath()
	        + GLShaderProgram::decodeStage(GLShaderProgram::Stage::FRAGMENT)
	              .first,
	    shader.getDefines(), _f);
	for(auto const& file : _f)
	{
		files.append(getAbsoluteDataPath("shaders/" + file));
	}
	files.removeDuplicates();

	auto t = new QTabWidget(this);
	layout->addWidget(t);

	for(auto const& f : files)
	{
		auto w         = new QWidget(this);
		auto tabLayout = new QVBoxLayout(w);

		QFile file(f);
		file.open(QIODevice::ReadOnly | QFile::Text);
		auto in   = new QTextStream(&file);
		auto text = new QTextEdit(this);
		text->setText(in->readAll().toLocal8Bit());
		delete in;
		file.close();
		tabLayout->addWidget(text);
		(void) new GlslSyntaxHighlighter(text->document());

		auto b = new QPushButton(tr("Save"), this);
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
	auto b = new QPushButton(tr("Reload"), this);
	connect(b, &QPushButton::pressed, [&shader]() { shader.reload(); });
	layout->addWidget(b);
}
