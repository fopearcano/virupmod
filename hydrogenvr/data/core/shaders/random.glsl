
// https://www.reedbeta.com/blog/hash-functions-for-gpu-rendering/
uvec4 _rng_state;

uvec4 rand4();
void srand(uint _seed)
{
	_rng_state.x = _seed;
	_rng_state.y = _seed + 1234u;
	_rng_state.z = _seed + 4567u;
	_rng_state.w = _seed + 8910u;
	_rng_state   = rand4();
}

uint rand()
{
	uint state   = _rng_state.x;
	_rng_state.x = _rng_state.x * 747796405u + 2891336453u;
	uint word    = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

uvec2 rand2()
{
	uvec2 state   = _rng_state.xy;
	_rng_state.xy = _rng_state.xy * 747796405u + 2891336453u;
	uvec2 word    = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

uvec3 rand3()
{
	uvec3 state    = _rng_state.xyz;
	_rng_state.xyz = _rng_state.xyz * 747796405u + 2891336453u;
	uvec3 word     = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

uvec4 rand4()
{
	uvec4 state = _rng_state;
	_rng_state  = _rng_state * 747796405u + 2891336453u;
	uvec4 word  = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

float randf()
{
	return float(rand()) * (1.0 / 4294967295.0);
}

vec2 randf2()
{
	return vec2(rand2()) * (1.0 / 4294967295.0);
}

vec3 randf3()
{
	return vec3(rand3()) * (1.0 / 4294967295.0);
}

vec4 randf4()
{
	return vec4(rand4()) * (1.0 / 4294967295.0);
}

float unif(float xmin, float xmax)
{
	return randf() * (xmax - xmin) + xmin;
}

vec2 unif2(vec2 xmin, vec2 xmax)
{
	return randf2() * (xmax - xmin) + xmin;
}

vec3 unif3(vec3 xmin, vec3 xmax)
{
	return randf3() * (xmax - xmin) + xmin;
}

vec4 unif4(vec4 xmin, vec4 xmax)
{
	return randf4() * (xmax - xmin) + xmin;
}

vec2 gaussian2(float mean, float standardDeviation)
{
	vec2 u = unif2(vec2(1.0e-5), vec2(1.0 - 1.0e-5));

	float R     = sqrt(-2.0 * log(u.x));
	float theta = 2.0 * 3.14159265359 * u.y;

	return mean + standardDeviation * vec2(R * cos(theta), R * sin(theta));
}

float gaussian(float mean, float standardDeviation)
{
	return gaussian2(mean, standardDeviation).x;
}

vec3 gaussian3(float mean, float standardDeviation)
{
	return vec3(gaussian2(mean, standardDeviation),
	            gaussian(mean, standardDeviation));
}

vec4 gaussian4(float mean, float standardDeviation)
{
	return vec4(gaussian2(mean, standardDeviation),
	            gaussian2(mean, standardDeviation));
}
