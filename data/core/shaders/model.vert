#version 150 core
#define LIGHTS_NB 2

in vec3 position;
in vec3 tangent;
in vec3 normal;
in vec2 texcoord;

uniform mat4 camera;

uniform mat4 lightspace[LIGHTS_NB];
uniform float boundingSphereRadius[LIGHTS_NB];
uniform mat4 localTransform;

out vec3 f_position;
out vec3 f_tangent;
out vec3 f_normal;
out vec2 f_texcoord;
out vec4 f_lightrelpos[LIGHTS_NB];

#include <shadows.glsl>

void main()
{
	gl_Position = camera * vec4(position, 1.0);

	f_position = position;
	f_tangent  = tangent;
	f_normal   = normal;
	f_texcoord = texcoord;
	for(int i = 0; i < LIGHTS_NB; ++i)
	{
		f_lightrelpos[i] = computeLightSpacePosition(
		    lightspace[i], (localTransform * vec4(position, 1.0)).xyz, normal,
		    boundingSphereRadius[i]);
	}
}
