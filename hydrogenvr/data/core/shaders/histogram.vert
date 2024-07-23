#version 150 core

in vec2 position;

uniform mat4 camera;

out vec2 texCoord;

void main()
{
	vec2 pos    = (camera * vec4(position, 0.0, 1.0)).xy;
	texCoord    = (position + vec2(1.0, 1.0)) / 2.0;
	gl_Position = vec4(pos, 0.0, 1.0);
}
