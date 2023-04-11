#version 150 core

in vec3 position;
in float radius;
in float luminosity;
in vec3 color; // in solar luminosity !

uniform mat4 camera;
uniform float alpha;
uniform vec3 campos;
uniform float pixelSolidAngle;
uniform float unitInKpc = 1.0;

uniform float pointSizeScale = 800.0; // empiric

uniform mat4 dusttransform;
uniform float useDust = 0.0;

uniform sampler3D dusttex;

out vec3 f_color;

const float oneOverLog10 = 0.4342944819;

out gl_PerVertex
{
	vec4 gl_Position;
	float gl_PointSize;
	float gl_ClipDistance[1];
};

float log10(in float x)
{
	return log(x) * oneOverLog10;
}

vec3 log10_3(in vec3 x)
{
	return log(x) * oneOverLog10;
}

#include <raymarch.glsl>

#include <gradient/gradient.glsl>

void main()
{
	vec4 pos           = camera * vec4(position, 1.0);
	gl_Position        = pos;
	gl_ClipDistance[0] = (pos.z / pos.w) - 0.1;

	vec3 usedColor = evaluateGradient(luminosity);

	float camdist = length(position - campos) * unitInKpc; // in kpc
	vec3 absmag = 4.83 - 2.5 * log10_3(max(vec3(1.0e-30), usedColor) ); // color is in Solar Luminosity ;
	                                           // sun is 4.83 abs mag
	vec3 apparentmag = absmag + 5.0 * (log10(camdist) + 2.0);
	vec3 irradiance  = pow(vec3(10.0), 0.4 * (-apparentmag - 14.0));
	vec3 luminance   = irradiance / pixelSolidAngle; // lum

	vec3 a = vec3(1.0);
	if(useDust == 1.0)
	{
		a = attenuation(campos, position, dusttex, dusttransform);
	}

	gl_PointSize = max(1.0, radius * pointSizeScale / camdist);

	f_color = a * alpha * luminance;

	// variable size and polynomical
	f_color /= pow(gl_PointSize, 2.0);

	// float grad = pow((luminosity - 0.2) * 10.0, 2.0);
	// f_color *= mix(vec3(0.1, 0.2, 1.0) * 1.0, vec3(1.0, 0.1, 0.0) * 1.0, grad);

	// float grad = min(1.0, max(0.0, (log(luminosity) / log(10.0) + 2.0) * 1.0));
	// float grad = min(1.0, max(0.0, (log(luminosity) / log(10.0) + 0.0) * 2.0));
	// f_color *= mix(vec3(0.1, 0.2, 1.0) * 0.1, vec3(1.0, 0.1, 0.0) * 10.0, grad);
}
