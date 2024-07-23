#version 150 core

in vec2 texCoord;

out vec4 outColor;
uniform sampler2D tex;
uniform float alpha = 1.0;
uniform float exposure = 1.0;
uniform float dynamicrange = 1.0;
uniform vec2 cursorPos = vec2(-1.0, -1.0);

#include <inv_exposure.glsl>

void main()
{
	outColor = texture(tex, texCoord);
	if(distance(texCoord, vec2(cursorPos.x, 1.0 - cursorPos.y)) < 0.01)
	{
		outColor += vec4(1.0, 0.0, 0.0, 0.0);
	}
	outColor.rgb *= outColor.a * alpha;
	outColor.rgb = inv_exposure(outColor.rgb, dynamicrange, exposure);
}
