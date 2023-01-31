#version 150 core

in vec3 f_color;

out vec4 outColor;

void main()
{
	float d = distance(gl_PointCoord, vec2(0.5, 0.5));
	if(d > 0.5) discard;

	const float p = 0.1;
	outColor = max(0.0, 1.0 - (pow(d, p) / pow(0.5, p))) * vec4(f_color, 1.0);
}
