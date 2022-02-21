/*
    Copyright (C) 2020 Florian Cabot <florian.cabot@hotmail.fr>

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
#ifndef SHADERPROGRAM_HPP
#define SHADERPROGRAM_HPP

#include <QString>
#include <map>
#include <unordered_set>

#include "gl/GLHandler.hpp"

typedef GLShaderProgram& GLSRef;

class ShaderProgram
{
  public:
	enum UniformBaseType
	{
		UNKNOWN,
		FLOAT,
		DOUBLE,
		UINT,
		INT,
	};

	ShaderProgram();
	ShaderProgram(QString const& shadersCommonName,
	              QMap<QString, QString> const& defines = {});
	ShaderProgram(QString const& vertexName, QString const& fragmentName,
	              QMap<QString, QString> const& defines = {});
	QString getVertexShaderPath() const { return vert; };
	QString getFragmentShaderPath() const { return frag; };
	QMap<QString, QString> getDefines() const { return defines; };
	void load(QString const& shadersCommonName,
	          QMap<QString, QString> const& defines = {});
	void load(QString const& vertexName, QString const& fragmentName,
	          QMap<QString, QString> const& defines = {});
	void reload();
	static void reloadAllShaderPrograms();
	void setUnusedAttributesValues(
	    std::vector<QPair<const char*, std::vector<float>>> const&
	        defaultValues) const
	{
		glShader->setUnusedAttributesValues(defaultValues);
	};
	void setUnusedAttributesValues(
	    QStringList const& names,
	    std::vector<std::vector<float>> const& values) const
	{
		glShader->setUnusedAttributesValues(names, values);
	};
	template <typename T>
	void setUniform(char const* name, T const& value) const;
	template <typename T>
	void setUniform(char const* name, unsigned int size, T const* value) const;
	QString toStr() const { return glShader->toStr(); };
	~ShaderProgram();

	static std::unordered_set<ShaderProgram*> const& getAllShaderPrograms()
	{
		return allShaderPrograms();
	};

	operator GLSRef() const;

  private:
	GLShaderProgram* glShader = nullptr;

	QString vert;
	QString frag;
	QMap<QString, QString> defines;

	mutable std::map<char const*, QVariant> uniformsBackup;

	static std::unordered_set<ShaderProgram*>& allShaderPrograms();

	// static std::pair<UniformBaseType, unsigned int> decodeUniformType(GLenum
	// type);
	GLShaderProgram& toGLShaderProgram() const { return *glShader; };
};

template <typename T>
void ShaderProgram::setUniform(char const* name, T const& value) const
{
	uniformsBackup[name] = value;
	glShader->setUniform(name, value);
}

template <typename T>
void ShaderProgram::setUniform(char const* name, unsigned int size,
                               T const* value) const
{
	QList<QVariant> list;
	for(unsigned int i(0); i < size; ++i)
	{
		list << value[i];
	}
	uniformsBackup[name] = list;
	glShader->setUniform(name, size, value);
}
#endif // SHADERPROGRAM_HPP
