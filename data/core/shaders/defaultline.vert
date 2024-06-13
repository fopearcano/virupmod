#version 150 core

in vec2 position;

uniform mat4 camera;

void main()
{
    gl_Position = camera*vec4(position, 0.0, 1.0);
}
