#version 450 core

#include <hvr/hvr.glsl>

in vec3 f_position;
uniform sampler2D creditsTex;
uniform vec3 color;
uniform float alpha        = 1.0;
uniform float aspectratio = 1.0;
out vec4 outColor;

void main()
{
	vec2 pos = vec2(f_position.y / aspectratio, -f_position.z);
	if(f_position.x > -0.499 || abs(f_position.y) > 0.49 || abs(f_position.z) > 0.49)
	{
		outColor = vec4(0.0);
	}
	else
	{
		outColor = texture(creditsTex, pos + vec2(0.5, 0.5));
	}
	outColor.rgb *= tmm.dynamicrange / tmm.exposure;
}
