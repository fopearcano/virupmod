#version 150 core

uniform vec3 color;
uniform float alpha = 1.0;
uniform float exposure = 1.0;
uniform float dynamicrange = 1.0;
out vec4 outColor;

#include <inv_exposure.glsl>

void main()
{
	outColor = vec4(color, alpha);
	outColor.rgb = inv_exposure(outColor.rgb, dynamicrange, exposure);
}
