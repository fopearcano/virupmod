in vec3 f_pos;

uniform int samples;
uniform float absorptionIntensity = 80.0;
uniform vec3 minbbox;
uniform vec3 maxbbox;
uniform vec3 campos;
uniform float time = 1.0; // for jitter
uniform sampler3D densityTex;

float random(vec2 uv)
{
	return fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453123) * 2.0
	       - 1.0;
}

float unif(vec2 uv, float xmin, float xmax)
{
	return (random(uv) * 0.5 + 0.5) * (xmax - xmin) + xmin;
}

vec3 stickToUnitBox(vec3 pos, vec3 dir)
{
	if(pos.x < 0.0)
	{
		pos -= dir * pos.x / dir.x;
	}
	if(pos.y < 0.0)
	{
		pos -= dir * pos.y / dir.y;
	}
	if(pos.z < 0.0)
	{
		pos -= dir * pos.z / dir.z;
	}
	if(pos.x > 1.0)
	{
		pos += dir * ((1.0 - pos.x) / dir.x);
	}
	if(pos.y > 1.0)
	{
		pos += dir * ((1.0 - pos.y) / dir.y);
	}
	if(pos.z > 1.0)
	{
		pos += dir * ((1.0 - pos.z) / dir.z);
	}
	return pos;
}

vec4 getTextureValue(vec3 coord)
{
	return texture(densityTex, coord);
}

vec3 posToCoord(vec3 pos)
{
	return (pos - minbbox) / (maxbbox - minbbox);
}

vec4 emissionAtPoint(vec3 position, vec3 coord);
vec4 absorptionAtPoint(vec3 position, vec3 coord);

vec4 att(vec4 absorptionSum)
{
	return pow(vec4(10.0), -absorptionIntensity * absorptionSum);
}

vec4 integrate(vec3 from, vec3 to)
{
	// stick to bbox if outside of bbox
	/*vec3 end = from;
	vec3 endUnitCube = (end - minbbox) / (maxbbox - minbbox);
	vec3 startUnitCube = (to - minbbox) / (maxbbox - minbbox);
	startUnitCube = stickToUnitBox(startUnitCube, normalize(endUnitCube - startUnitCube));
	vec3 start = minbbox + startUnitCube * (maxbbox - minbbox);

	from = start;
	to = end;*/

	vec4 result = vec4(0.0);
	vec4 absorptionSum = vec4(0.0);
	vec3 ds = (to - from) / float(samples - 1);
	float lds = length(ds);

	vec3 jitter;
	jitter.x = unif(vec2(to.x, time), 0.0, 1.0);
	jitter.y = unif(vec2(time, to.y), 0.0, 1.0);
	jitter.z = unif(vec2(to.z, time), 0.0, 1.0);
	jitter *= 0.002; // Adjust jitter magnitude as needed
	// return vec4(jitter, 1.0);
	vec3 p = from + jitter;
	for(int i = 0; i < samples; i++)
	{
		vec3 coord = posToCoord(p);
		absorptionSum += lds * absorptionAtPoint(p, coord);
		result += emissionAtPoint(p, coord) * att(absorptionSum);
		p += ds;
	}
	return result * lds;
}

vec4 raymarch()
{
	return integrate(campos, f_pos);
}

