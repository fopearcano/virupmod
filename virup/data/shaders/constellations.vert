#version 150 core

in vec3 position;

uniform mat4 camera;

uniform vec3 camPos;
uniform float unit;

out float dist;
out float vertexID;

void main()
{
    gl_Position = camera*vec4(position, 1.0);

	dist = length(camPos - position*unit);

	vertexID = gl_VertexID;
}
