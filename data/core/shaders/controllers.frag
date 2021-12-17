#version 150 core

uniform vec3 color;
uniform sampler2D tex;

uniform float exposure = 1.0;
uniform float dynamicrange = 1.0;

in vec3 f_normal;
in vec2 f_texcoord;

out vec4 outColor;

#include <inv_exposure.glsl>

void main()
{
	outColor = texture(tex, f_texcoord);
	outColor.rgb = inv_exposure(outColor.rgb, dynamicrange, exposure);
}
