#version 460
#define M_PI 3.1415926535897932384626433832795

layout(binding = 0) uniform sampler2D inputTex;

uniform int blurRadius;
uniform float sigmaI;
uniform float sigmaS;

smooth in vec2 texCoord;

out float fragColor;

float gaussian(float x, float sigma)
{
	float a = 2.0f * sigma * sigma;
    return exp(-(x * x) / a) / (M_PI * a);
}

float getDepth(vec2 texCoord)
{
	return texture(inputTex, texCoord).r;
}

void main()
{
    float depth = getDepth(texCoord);
    vec2 dim = textureSize(inputTex, 0);

	float sum = 0.0f;
	float weightSum = 0.0f;
	for (int j = -blurRadius; j < blurRadius; j++)
	{
		for (int i = -blurRadius; i < blurRadius; i++)
		{
			vec2 dx = vec2(float(i), float(j)) / dim;
			vec2 pos = texCoord + dx;

			// Gaussian weight given the difference of intensity
			float currDepth = getDepth(pos);
			float gi = gaussian(currDepth - depth, sigmaI);

			// Gaussian weight given the difference in position
			float dist = sqrt(dx.x * dx.x + dx.y * dx.y);
			float gs = gaussian(dist, sigmaS);

			// Then convolve
			float w = gi * gs;
			sum += currDepth * w;
			weightSum += w;
		}
	}
    fragColor = sum / weightSum * 2.0f;
}