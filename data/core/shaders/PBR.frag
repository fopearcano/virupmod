#version 420 core
#define LIGHTS_NB 2

uniform samplerCube irradiance;
uniform samplerCube prefiltered;
uniform sampler2D brdfLUT;
uniform sampler2D albedo;
uniform sampler2D occlusion;
uniform sampler2D emissive;
uniform sampler2D metallicRoughness;
uniform sampler2D normalTex;
uniform sampler2DShadow shadowmap[LIGHTS_NB];

uniform float occlusionBase;
uniform float occlusionStrength;
uniform vec3 emissiveBase;
uniform vec3 emissiveFactor;
uniform vec2 metallicRoughnessBase;

uniform vec3 lightDirection[LIGHTS_NB];
uniform vec3 lightColor[LIGHTS_NB];
uniform float lightAmbiantFactor[LIGHTS_NB];

uniform mat4 model;
uniform vec3 campos;

uniform int debug = 0;

in vec3 f_position;
in vec3 f_normal;
in vec4 f_tangent;
in vec3 f_color_0;
in vec2 f_texcoord_0;
in vec4 f_lightrelpos[LIGHTS_NB];

out vec4 outColor;

const float PI = 3.14159265359;

vec2 posToCoord(vec3 pos)
{
	if(length(f_texcoord_0 - vec2(-1.42069)) < 0.00001)
	{
		float lat = asin(pos.z);
		float lon = atan(pos.y, pos.x);

		return vec2(0.5 * lon / PI + 0.5, lat / PI + 0.5);
	}
	return f_texcoord_0;
}

vec3 getAlbedo(vec3 pos)
{
#ifdef TEXTURED_ALBEDO
	return texture(albedo, posToCoord(pos)).rgb;

#else
	return f_color_0;
#endif
}

float getOcclusion(vec3 pos)
{
#ifdef TEXTURED_OCCLUSION
	float ao = texture(occlusion, posToCoord(pos)).r;
#else
	float ao = occlusionBase;
#endif
	return 1.0 + occlusionStrength * (ao - 1.0);
}

vec3 getEmissive(vec3 pos)
{
#ifdef TEXTURED_EMISSIVE
	vec3 em = texture(emissive, posToCoord(pos)).rgb;
#else
	vec3 em  = emissiveBase;
#endif
	return em * emissiveFactor;
}

vec2 getMetallicRoughness(vec3 pos)
{
#ifdef TEXTURED_METALLICROUGHNESS
	return texture(metallicRoughness, posToCoord(pos)).bg;

#else
	return metallicRoughnessBase;
#endif
}

vec4 getTangent(vec3 pos)
{
	return vec4(normalize(f_tangent.xyz), f_tangent.w);
}

#ifdef TEXTURED_NORMAL
vec3 getNormal(vec3 pos)
{
	vec3 normal    = normalize(f_normal);
	vec3 tangent   = normalize(f_tangent.xyz);

	if(!gl_FrontFacing)
	{
		normal *= -1.0;
		tangent *= -1.0;
	}

	vec3 bitangent = normalize(f_tangent.w * cross(normal, tangent));

	vec3 localNormal = texture(normalTex, posToCoord(pos)).rgb * 2.0 - 1.0;
	vec3 modelNormal
	    = normalize(localNormal.x * tangent + localNormal.y * bitangent
	                + localNormal.z * normal);
	// world normal
	return normalize((model * vec4(modelNormal, 0.0)).xyz);
}
#else
vec3 getNormal(vec3 pos)
{
	vec3 normal = f_normal;
	if(!gl_FrontFacing)
	{
		normal *= -1.0;
	}
	return normalize((model * vec4(normalize(normal), 0.0)).xyz);
}
#endif

#include <shadows.glsl>

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
	return F0
	       + (max(vec3(1.0 - roughness), F0) - F0)
	             * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a      = roughness * roughness;
	float a2     = a * a;
	float NdotH  = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float num   = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom       = PI * denom * denom;

	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float num   = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2  = GeometrySchlickGGX(NdotV, roughness);
	float ggx1  = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

void main()
{
	vec3 worldpos = (model * vec4(f_position, 1.0)).xyz;
	vec3 V        = normalize(campos - worldpos);
	vec3 N        = normalize(getNormal(f_position));

	vec3 albedo            = getAlbedo(f_position);
	vec2 metallicRoughness = getMetallicRoughness(f_position);

	vec3 F0       = vec3(0.04);
	F0            = mix(F0, albedo, metallicRoughness.x);

	// DIRECT LIGHTING
	outColor = vec4(0.0, 0.0, 0.0, 1.0);
	for(int i = 0; i < LIGHTS_NB; ++i)
	{
		if(lightColor[i] == vec3(0.0))
		{
			continue;
		}
		float shadow = computeShadow(f_lightrelpos[i], shadowmap[i]);
		if(debug == 1)
		{
			// shadow = 1.0;
		}
		vec3 radiance = shadow * lightColor[i];

		vec3 L = -lightDirection[i];

		vec3 H = normalize(V + L);

		vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

		float NDF = DistributionGGX(N, H, metallicRoughness.y);
		float G   = GeometrySmith(N, V, L, metallicRoughness.y);

		vec3 numerator = NDF * G * F;
		float denominator
		    = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
		vec3 specular = numerator / denominator;

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallicRoughness.x;

		float NdotL = max(dot(N, L), 0.0);
		vec3 Lo     = (kD * albedo / PI + specular) * radiance * NdotL;

		outColor += vec4(Lo, 1.0);
	}

	// INDIRECT LIGHTING
	vec3 F
	    = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, metallicRoughness.y);
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallicRoughness.x;

	// diffuse
	vec3 indirectLighting = kD * texture(irradiance, N).rgb * albedo;

	// specular
	vec3 R = reflect(-V, N);

	const float MAX_REFLECTION_LOD = 4.0;
	float roughness                = metallicRoughness.y;
	// !!!!! ONLY FOR RUSTED PANELS
	if(debug != 1)
	{
		roughness = pow(roughness, 2.7);
	}
	vec3 prefilteredColor
	    = textureLod(prefiltered, R, roughness * MAX_REFLECTION_LOD).rgb;
	vec2 envBRDF
	    = texture(brdfLUT, vec2(max(dot(N, V), 0.0), metallicRoughness.y)).rg;
	vec3 specular = prefilteredColor
	           * (F * max(0.0, envBRDF.x) + max(vec3(0.0), vec3(envBRDF.y)));

	indirectLighting += kS * specular;

	indirectLighting *= getOcclusion(f_position);

	outColor.rgb += indirectLighting + getEmissive(f_position);
}
