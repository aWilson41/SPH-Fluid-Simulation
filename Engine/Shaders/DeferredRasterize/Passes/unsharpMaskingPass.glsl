#version 460
#define M_PI 3.1415926535897932384626433832795

layout(binding = 0) uniform sampler2D colorTex;
layout(binding = 1) uniform sampler2D depthTex;

uniform int blurRadius;
uniform float sigma;
uniform float darknessFactor;

smooth in vec2 texCoord;

out vec4 fragColor;

float gaussian(float x, float sigma)
{
	float a = 2.0f * sigma * sigma;
    return exp(-(x * x) / a) / (M_PI * a);
}

void main()
{
    vec3 color = texture(colorTex, texCoord).rgb;
    float depth = texture(depthTex, texCoord).r;

    if (depth > 1000.0f)
        fragColor = vec4(color, 1.0f);
    else
    {
        float blurDepth = 0.0f;
        float weight = 0.0f;
        float sum = 0.0f;
        vec2 dim = textureSize(depthTex, 0);
        for (int i = -blurRadius; i <= blurRadius; i++)
        {
            for (int j = -blurRadius; j <= blurRadius; j++)
            {
                vec2 dx = vec2(float(i), float(j)) / dim;
				vec2 pos = texCoord + dx;

                float currDepth = texture(depthTex, pos).r;
                float r = sqrt(float(i * i + j * j));
                float weight = gaussian(r, sigma);
                blurDepth += currDepth * weight;
                sum += weight;
            }
        }
        float delta = darknessFactor * max(depth - blurDepth / sum, 0.0f);
        fragColor = vec4(color - delta, 1.0f);
    }
}