#version 150 core

in vec3 f_position;

uniform sampler2D tex;
uniform float brightness;

uniform float PI = 3.14159265359;

out vec4 outColor;

void main()
{
	vec3 pos = normalize(f_position);
	vec2 texCoord
	    = vec2(atan(pos.y, pos.x) / (2.0 * PI), (asin(pos.z) / PI) + 0.5);

	texCoord.x -= 0.5; // center of texture is longitude 0

	if(texCoord.x < 0.0)
	{
		texCoord.x += 1.0;
	}
	outColor = texture(tex, vec2(1.0) - texCoord);

	outColor.rgb *= brightness;
	outColor.a = 1.0;
}

