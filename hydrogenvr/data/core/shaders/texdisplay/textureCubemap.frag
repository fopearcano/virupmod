#version 150 core

in vec2 texCoord;

out vec4 outColor;

uniform samplerCube tex;

void main()
{
	vec2 lonlat = 3.1415 * vec2(1.0, 0.5) * (texCoord * 2.0 - 1.0);

	vec3 n = vec3(cos(lonlat.x) * cos(lonlat.y), sin(lonlat.x) * cos(lonlat.y), sin(lonlat.y));

	outColor = texture(tex, n);
	return;

	ivec2 grid = ivec2(floor(texCoord * 20.0)) % ivec2(2);
	float gridCol = (grid.x + grid.y) % 2;
	vec3 alphaCol = gridCol * vec3(0.8) + (1.0 - gridCol) * vec3(0.3);
	outColor.rgb = mix(alphaCol, outColor.rgb, outColor.a);
}
