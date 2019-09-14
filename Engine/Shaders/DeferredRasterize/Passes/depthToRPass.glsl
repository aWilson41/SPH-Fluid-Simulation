// Linearizes the depth buffer and outputs it
#version 460

layout(binding = 0) uniform sampler2D depthTex;

uniform float nearZ = 0.0f;
uniform float farZ = 1.0f;

smooth in vec2 texCoord;

out float fragColor;

float linearizeDepth(float z)
{
	return (2.0f * nearZ * farZ) / (farZ + nearZ - z * (farZ - nearZ));
}

void main()
{
	fragColor = linearizeDepth(texture(depthTex, texCoord).r);
}