#version 430 core

in vec2 texCoord;

uniform vec3 color;

out vec4 outColor;

layout(std430, binding = 0) buffer HistBuffer
{
	int values[];
};

uniform int bufferSize;

void main()
{
	int maxVal = -1;
	for(int i = 0; i < bufferSize; ++i)
	{
		if(values[i] > maxVal)
			maxVal = values[i];
	}
	int idx = int(floor(bufferSize * texCoord.x));
	float f = fract(bufferSize * texCoord.x);
	float relVal = values[int(bufferSize * texCoord.x)] / float(maxVal);
	outColor = vec4(color * sqrt((1.0 - f)* 0.5 + 0.5), step(texCoord.y, relVal));
}
