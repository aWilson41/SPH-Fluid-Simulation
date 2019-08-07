#version 460
#define M_PI 3.1415926535897932384626433832795

layout(binding = 0) uniform sampler2D gColor;
layout(binding = 1) uniform sampler2D gDepth;

uniform int blurRadius;
uniform float sigma;
uniform float darknessFactor;
uniform float nearZ;
uniform float farZ;

smooth in vec2 texCoord;

out vec4 fColor;

// float linearizeDepth(float z)
// {
//   return (2.0f * nearZ) / (farZ + nearZ - z * (farZ - nearZ));
// }

void main()
{
    vec3 color = texture(gColor, texCoord).rgb;
    float depth = texture(gDepth, texCoord).r;

    if (depth >= 0.9999f)
        fColor = vec4(color, 1.0f);
    else
    {
        float blurDepth = 0.0f;
        //float sigma = 2.0f * blurRadius * blurRadius;
        //float currentDepth;//, dist, gauss;
        //vec2 pos;
        float s = 2.0f * sigma * sigma;
        float weight = 0.0f;
        float sum = 0.0f;
        vec2 dim = textureSize(gDepth, 0);
        for (int i = -blurRadius; i <= blurRadius; i++)
        {
            for (int j = -blurRadius; j <= blurRadius; j++)
            {
                vec2 pos = texCoord + vec2(float(i) / dim.x, float(j) / dim.y);
                //pos.x = texCoord.x + i;// * pixelScale[0];
                //pos.y = texCoord.y + j;// * pixelScale[1];
                float d = texture(gDepth, pos).r;
                float r = sqrt(float(i * i + j * j));
                float weight = exp(-(r * r) / s) / (M_PI * s);
                blurDepth += d * weight;
                sum += weight;
            }
        }
        float delta = darknessFactor * max(depth - blurDepth / sum, 0.0f);
        //fColor = vec4(blurDepth, blurDepth, blurDepth, 1.0f);
        fColor = vec4(color - delta, 1.0f);
    }
}

// blurDepth is a lot less than depth when neighboring pixels have large differences in depths