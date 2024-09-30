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

#include "gl/GLHandler.hpp"
#include <algorithm>

#include "gl/GLComputeShader.hpp"

GLComputeShader::GLComputeShader(QString const& fileName,
                                 QMap<QString, QString> const& defines)
    : GLShaderProgram({{fileName, Stage::COMPUTE}}, addDefines(defines))
{
	get(GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize.data());
}

void GLComputeShader::exec(std::vector<TextureBinding> const& textureBindings,
                           std::array<unsigned int, 3> const& globalGroupSize,
                           bool waitForFinish) const
{
	use();

	for(unsigned int i(0); i < textureBindings.size(); ++i)
	{
		auto const& binding(textureBindings[i]);
		if(binding.accessMode == DataAccessMode::SAMPLER)
		{
			binding.texture.use(i);
			continue;
		}
		binding.texture.useAsImage(i, binding.level, binding.accessMode);
	}
	std::array<GLint, 3> dispatchSize{};
	std::ranges::transform(globalGroupSize, workGroupSize, dispatchSize.begin(),
	                       [](int globGrSize, int workGrSize) {
		                       return globGrSize / workGrSize
		                              + ((globGrSize % workGrSize == 0) ? 0
		                                                                : 1);
	                       });

	GLHandler::glf().glDispatchCompute(dispatchSize[0], dispatchSize[1],
	                                   dispatchSize[2]);

	if(waitForFinish)
	{
		// add GL_SHADER_STORAGE_BARRIER_BIT for SSBO later
		GLHandler::glf().glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
}

QMap<QString, QString>
    GLComputeShader::addDefines(QMap<QString, QString> const& userDefines)
{
	QMap<QString, QString> result(userDefines);

	GLint maxInvocations = 0;
	GLHandler::glf().glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS,
	                               &maxInvocations);

	result["LOCAL_SIZE_1D_X"] = QString::number(maxInvocations);
	result["LOCAL_SIZE_2D_X"] = QString::number(floor(sqrt(maxInvocations)));
	result["LOCAL_SIZE_2D_Y"] = QString::number(floor(sqrt(maxInvocations)));
	result["LOCAL_SIZE_3D_X"] = QString::number(floor(cbrt(maxInvocations)));
	result["LOCAL_SIZE_3D_Y"] = QString::number(floor(cbrt(maxInvocations)));
	result["LOCAL_SIZE_3D_Z"] = QString::number(floor(cbrt(maxInvocations)));

	return result;
}
