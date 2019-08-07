// Basically a directional blur in order to preserve edges 
#version 460

layout(binding = 0) uniform sampler2D depthTex;

uniform float blurRadius;
uniform float blurScale;
uniform float blurDepthFalloff;
uniform vec2 blurDir;

smooth in vec2 texCoord;

layout (depth_any) out float gl_FragDepth;

void main()
{
    float depth = texture(depthTex, texCoord).r;
    float sum = 0.0f;
    float wsum = 0.0f;
    for (float x = -blurRadius; x <= blurRadius; x += 1.0f)
    {
        float currDepth = texture(depthTex, texCoord + x * blurDir).r;
        // Spatial domain
        float r = x * blurScale;
        float w = exp(-r * r);
        // Range domain
        float r2 = (currDepth - depth) * blurDepthFalloff;
        float g = exp(-r2 * r2);
        sum += currDepth * w * g;
        wsum += w * g;
    }
    if (wsum > 0.0f)
        sum /= wsum;
    gl_FragDepth = sum;
    //gl_FragDepth = depth;
}