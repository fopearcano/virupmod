vec3 inv_exposure(vec3 color, float dynamicrange, float exposure)
{
	vec3 result = color;
	if(dynamicrange > 1.0)
	{
		float inv_b = log(dynamicrange) / log(2.0 * 255.0);
		result.rgb = pow(result.rgb, vec3(inv_b));
		result.rgb *= dynamicrange;
		result.rgb /= exposure;
	}
	return result;
}
