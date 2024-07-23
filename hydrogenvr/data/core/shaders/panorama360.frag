#version 150 core

in vec2 texCoord;

out vec4 outColor;

uniform samplerCube tex;


void main()
{
	const float pi = 3.14159265359;
	vec2 lonlat = (texCoord-0.5) * vec2(2.0*pi, pi);

	vec3 pos = vec3(cos(lonlat.x) * cos(lonlat.y), sin(lonlat.x)*cos(lonlat.y), sin(lonlat.y));

	vec3 pos2 = pos.yzx;
	pos2.z *= -1;

	pos2 = normalize(pos2);

	float maxCos = -1.0;
	if(dot(vec3(1.0, 0.0, 0.0), pos2) > maxCos)
		maxCos = dot(vec3(1.0, 0.0, 0.0), pos2);
	if(dot(vec3(-1.0, 0.0, 0.0), pos2) > maxCos)
		maxCos = dot(vec3(-1.0, 0.0, 0.0), pos2);
	if(dot(vec3(0.0, 1.0, 0.0), pos2) > maxCos)
		maxCos = dot(vec3(0.0, 1.0, 0.0), pos2);
	if(dot(vec3(0.0, -1.0, 0.0), pos2) > maxCos)
		maxCos = dot(vec3(0.0, -1.0, 0.0), pos2);
	if(dot(vec3(0.0, 0.0, 1.0), pos2) > maxCos)
		maxCos = dot(vec3(0.0, 0.0, 1.0), pos2);
	if(dot(vec3(0.0, 0.0, -1.0), pos2) > maxCos)
		maxCos = dot(vec3(0.0, 0.0, -1.0), pos2);


	outColor = texture(tex, pos2) / maxCos;
}
