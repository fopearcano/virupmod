#version 150

in vec2 texCoord;

out vec4 outColor;

uniform sampler2D tex;

uniform vec2 scale  = vec2(1.0, 1.0);
uniform float alpha = 1.0;

void main()
{
	vec2 coord = texCoord;

	// rescale at center
	coord -= vec2(0.5);
	coord *= scale;
	coord += vec2(0.5);
	coord.y = 1.0 - coord.y; // invert Y

	outColor = texture(tex, coord);
	outColor.a *= alpha;
}
