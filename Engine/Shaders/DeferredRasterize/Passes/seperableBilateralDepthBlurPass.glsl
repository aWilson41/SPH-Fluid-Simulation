#version 460
#define M_PI 3.1415926535897932384626433832795

layout(binding = 0) uniform sampler2D depthTex;

uniform vec2 blurDir;
uniform int blurRadius;
uniform float sigmaI;
uniform float sigmaS;

smooth in vec2 texCoord;

layout (depth_any) out float gl_FragDepth;

float gaussian(float x, float sigma)
{
	float a = 2.0f * sigma * sigma;
    return exp(-(x * x) / a) / (M_PI * a);
}

void main()
{
    float depth = texture(depthTex, texCoord).r;
    vec2 dim = textureSize(depthTex, 0);
	vec2 texelSize = 1 / dim;

	float sum = 0.0f;
	float weightSum = 0.0f;
	for (int i = -blurRadius; i < blurRadius; i++)
	{
		vec2 dx = float(i) * blurDir * texelSize;
		vec2 pos = texCoord + dx;

		float currDepth = texture(depthTex, pos).r;
		float gi = gaussian(currDepth - depth, sigmaI);

		float dist = sqrt(dx.x * dx.x + dx.y * dx.y);
		float gs = gaussian(dist, sigmaS);
		float w = gi * gs;
		sum += currDepth * w;
		weightSum += w;
	}
	//if (weightSum > 0.0f)
        sum /= weightSum;
    gl_FragDepth = sum;
}