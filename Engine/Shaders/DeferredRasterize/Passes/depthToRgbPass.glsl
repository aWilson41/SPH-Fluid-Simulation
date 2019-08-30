#version 460

layout(binding = 0) uniform sampler2D depthTex;

uniform float nearZ = 0.0f;
uniform float farZ = 1.0f;

smooth in vec2 texCoord;

out vec4 fragColor;

float linearizeDepth(float z)
{
	return (2.0f * nearZ * farZ) / (farZ + nearZ - z * (farZ - nearZ));
}

void main()
{
    float depth = linearizeDepth(texture(depthTex, texCoord).r);
	fragColor = vec4(depth, depth, depth, 1.0f);
}