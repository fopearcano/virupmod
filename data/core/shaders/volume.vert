#version 150 core

in vec3 position;

uniform mat4 camera;
uniform vec3 minbbox;
uniform vec3 maxbbox;

out vec3 f_pos;

void main()
{
	vec3 pos = minbbox + (position + 0.5) * (maxbbox - minbbox);
	gl_Position = camera*vec4(pos, 1.0);
	f_pos = pos;
}
