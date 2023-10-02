#version 150 core

out vec4 outColor;

#include <volume.glsl>

vec4 emissionAtPoint(vec3 position, vec3 coord)
{
	// return pow(vec4(cos(coord), 1.0), vec4(10.0)) * 1.0e-1;
	return vec4(getTextureValue(coord).r);
}

vec4 absorptionAtPoint(vec3 position, vec3 coord)
{
	// return vec4(1.0 - step(0.1, length(coord - vec3(0.5))));
	return vec4(getTextureValue(coord).g);
}

void main()
{
	outColor = vec4(raymarch().xyz * vec3(10.0), 1.0);
}
