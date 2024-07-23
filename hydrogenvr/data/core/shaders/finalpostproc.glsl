
uniform float gamma;
uniform float contrast = 1.0;

#ifdef DITHERING

const int bayer_pattern[64]
    = int[](0, 32, 8, 40, 2, 34, 10, 42,    // 8x8 Bayer ordered dithering
            48, 16, 56, 24, 50, 18, 58, 26, // pattern.  Each input pixel
            12, 44, 4, 36, 14, 46, 6, 38,   // is scaled to the 0..63 range
            60, 28, 52, 20, 62, 30, 54, 22, // before looking in this table
            3, 35, 11, 43, 1, 33, 9, 41,    // to determine the action.
            51, 19, 59, 27, 49, 17, 57, 25, 15, 47, 7, 39, 13, 45, 5, 37, 63,
            31, 55, 23, 61, 29, 53, 21);

vec3 dither()
{
	int x = int(mod(round(float(gl_GlobalInvocationID.x) / 1.5), 8.0));
	int y = int(mod(round(float(gl_GlobalInvocationID.y) / 1.5), 8.0));
	return vec3(bayer_pattern[x + 8 * y] / (32.0 * 256.0) - (1.0 / 128.0));
}

#endif

float max3(vec3 v)
{
	return max(max(v.x, v.y), v.z);
}

vec3 finalpostproc(vec3 color)
{
	float m = max3(color);
	if(m > 1.0)
	{
		color /= m;
	}
	color = pow(color, vec3(1.0 / gamma));

#ifdef DITHERING
	// dithering to remove banding
	color += dither();
#endif

	// contrast
	color.rgb
	    = clamp(contrast * (color.rgb - 0.5) + 0.5, vec3(0.0), vec3(1.0));

	return color;
}
