#version 150 core
#define LIGHTS_NB 2

in vec3 position;
in vec3 normal;
in vec4 tangent;
in vec3 color_0;
in vec2 texcoord_0;

uniform mat4 camera;

uniform mat4 lightspace[LIGHTS_NB];
uniform vec3 lightDirection[LIGHTS_NB];
uniform float boundingSphereRadius[LIGHTS_NB];
uniform mat4 model;

out vec3 f_position;
out vec3 f_normal;
out vec4 f_tangent;
out vec3 f_color_0;
out vec2 f_texcoord_0;
out vec4 f_lightrelpos[LIGHTS_NB];

#include <shadows.glsl>

void main()
{
	gl_Position = camera * vec4(position, 1.0);
	f_position  = position;
	vec3 N      = normalize(position + vec3(0.0, 0.0, 0.001));
	if(length(normal) > 0.0)
		N = normal;
	vec4 T = vec4(normalize(cross(vec3(0.0, 0.0, 1.0), N)), 1.0);
	if(length(tangent.xyz) < 10.0)
		T = tangent;
	f_normal     = N;
	f_tangent    = T;
	f_color_0    = color_0;
	f_texcoord_0 = texcoord_0;
	for(int i = 0; i < LIGHTS_NB; ++i)
	{
		f_lightrelpos[i] = computeLightSpacePosition(
		    lightspace[i], (model * vec4(position, 1.0)).xyz,
		    -lightDirection[i], boundingSphereRadius[i]);
	}
}
