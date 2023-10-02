#version 150 core

in vec2 texCoord;

uniform sampler2D tex;

out vec4 outColor;

#include <inv_exposure.glsl>

void main()
{
	outColor = texture(tex, texCoord);
}
