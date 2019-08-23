#version 460
#define M_PI 3.1415926535897932384626433832795

layout(binding = 0) uniform sampler2D inputTex;

uniform int blurRadius;
uniform float sigmaI;
uniform float sigmaS;

smooth in vec2 texCoord;

out vec3 fragColor;

float gaussian(float x, float sigma)
{
	float a = 2.0f * sigma * sigma;
    return exp(-(x * x) / a) / (M_PI * a);
}

void main()
{
    vec3 srcRgb = texture(inputTex, texCoord).rgb;
    vec2 dim = textureSize(inputTex, 0);

	vec3 sum = vec3(0.0f);
	vec3 weightSum = vec3(0.0f);
	for (int j = -blurRadius; j < blurRadius; j++)
	{
		for (int i = -blurRadius; i < blurRadius; i++)
		{
			vec2 dx = vec2(float(i), float(j)) / dim;
			vec2 pos = texCoord + dx;
			vec3 currRgb = texture(inputTex, pos).rgb;

			vec3 gi = vec3(
				gaussian(currRgb.r - srcRgb.r, sigmaI),
				gaussian(currRgb.g - srcRgb.g, sigmaI),
				gaussian(currRgb.b - srcRgb.b, sigmaI));

			float dist = sqrt(dx.x * dx.x + dx.y * dx.y);
			float gs = gaussian(dist, sigmaS);
			vec3 w = gi * gs;
			sum += currRgb * w;
			weightSum += w;
		}
	}
	if (weightSum.r > 0.0f)
        sum.r /= weightSum.r;
	if (weightSum.g > 0.0f)
        sum.g /= weightSum.b;
	if (weightSum.b > 0.0f)
        sum.b /= weightSum.b;
    fragColor = sum;
}