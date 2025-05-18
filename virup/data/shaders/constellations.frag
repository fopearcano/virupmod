#version 450 core

#include <hvr/hvr.glsl>

in float dist;
in float vertexID;
uniform vec3 color;
uniform float alpha = 1.0;
out vec4 outColor;

void main()
{
	float coeff = pow(clamp(dist*1000.0, 0.0, 1.0), 20.0);
	// variable alpha (high near star, low on middle of edge)
	// coeff *= pow(abs(fract(vertexID) - 0.5)*2.0, 5.0)*0.9 + 0.1;
	outColor = vec4(color, alpha * coeff);
	outColor.rgb *= tmm.dynamicrange / tmm.exposure;
}
