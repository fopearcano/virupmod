#version 150 core

in vec2 texCoord;

out vec4 outColor;

uniform sampler2D tex;

void main()
{
	outColor = texture(tex, texCoord);

	ivec2 grid = ivec2(floor(texCoord * 20.0)) % ivec2(2);
	float gridCol = (grid.x + grid.y) % 2;
	vec3 alphaCol = gridCol * vec3(0.8) + (1.0 - gridCol) * vec3(0.3);
	outColor.rgb = mix(alphaCol, outColor.rgb, outColor.a);
}
