#version 420 core
#define LIGHTS_NB 2

uniform sampler2D diffuse;
uniform sampler2D specular;
uniform sampler2D ambient;
uniform sampler2D emissive;
uniform sampler2D normals;
uniform sampler2D shininess;
uniform sampler2D opacity;
uniform sampler2D lightmap;

uniform sampler2DShadow shadowmap[LIGHTS_NB];

in vec3 f_position;
in vec3 f_tangent;
in vec3 f_normal;
in vec2 f_texcoord;
in vec4 f_lightrelpos[LIGHTS_NB];

uniform vec3 lightDirection[LIGHTS_NB];
uniform vec3 lightColor[LIGHTS_NB];
uniform float lightAmbiantFactor[LIGHTS_NB];

uniform mat4 model;
uniform vec3 campos;

out vec4 outColor;

#include <shadows.glsl>

void main()
{
	mat3 fromtangentspace;
	fromtangentspace[2] = normalize(f_normal);
	if(length(f_tangent) == 0.0)
	{
		fromtangentspace[0] = cross(vec3(0.0, 0.0, 1.0), fromtangentspace[2]);
	}
	else
	{
		fromtangentspace[0] = normalize(f_tangent);
	}

	// Gram-Schmidt re-orthogonalize T with respect to N
	fromtangentspace[0] = normalize(
	    fromtangentspace[0]
	    - dot(fromtangentspace[0], fromtangentspace[2]) * fromtangentspace[2]);

	fromtangentspace[1] = cross(fromtangentspace[2], fromtangentspace[0]);

	vec4 diffuseColor   = texture(diffuse, f_texcoord);
	vec4 specularColor  = texture(specular, f_texcoord);
	vec4 ambientColor   = texture(ambient, f_texcoord);
	vec4 emissiveColor  = texture(emissive, f_texcoord);
	vec4 normalColor    = texture(normals, f_texcoord);
	vec4 shininessColor = texture(shininess, f_texcoord);
	vec4 opacityColor   = texture(opacity, f_texcoord);
	vec4 lightmapColor  = texture(lightmap, f_texcoord);

	vec3 normal = normalize(fromtangentspace * (normalColor.rgb * 2.0 - 1.0));
	normal = normalize((model * vec4(normal, 0.0)).xyz); // to world
	vec3 viewDir    = normalize(campos - (model * vec4(f_position, 1.0)).xyz);

	outColor = vec4(0.0, 0.0, 0.0, 1.0);
	for(int i = 0; i < LIGHTS_NB; ++i)
	{
		// todo use normalmap
		float lightcoeff = max(0.0, dot(normal, -1.0 * lightDirection[i]));

		// shadow map
		float shadow = computeShadow(f_lightrelpos[i], shadowmap[i]);
		lightcoeff *= shadow;

		// diffuse
		vec3 Lo = max(lightAmbiantFactor[i], lightcoeff) * diffuseColor.rgb
		          * lightColor[i];

		// specular
		vec3 reflectDir = reflect(lightDirection[i], normal);
		float spec      = pow(max(dot(viewDir, reflectDir), 0.0), 32);
		Lo += spec * lightColor[i] * max(0.0, lightcoeff) * specularColor.rgb;
		Lo /= 2.0;
		outColor.rgb += Lo;
	}

	outColor.rgb += emissiveColor.rgb;

	// transparency
	outColor.a = 1.0;

	// lightmap
	outColor.rgb *= lightmapColor.r;

}
