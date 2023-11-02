
// https://www.reedbeta.com/blog/hash-functions-for-gpu-rendering/
uvec4 _rng_state;

uvec4 rand4();
void srand(uint _seed)
{
	_rng_state.x = _seed;
	_rng_state.y = _seed + 1234;
	_rng_state.z = _seed + 4567;
	_rng_state.w = _seed + 8910;
	_rng_state = rand4();
}

uint rand()
{
	uint state = _rng_state.x;
	_rng_state.x = _rng_state.x * 747796405u + 2891336453u;
	uint word  = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

uvec2 rand2()
{
	uvec2 state = _rng_state.xy;
	_rng_state.xy = _rng_state.xy * 747796405u + 2891336453u;
	uvec2 word  = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

uvec3 rand3()
{
	uvec3 state = _rng_state.xyz;
	_rng_state.xyz = _rng_state.xyz * 747796405u + 2891336453u;
	uvec3 word  = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

uvec4 rand4()
{
	uvec4 state = _rng_state;
	_rng_state = _rng_state * 747796405u + 2891336453u;
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
