#version 460

layout(binding = 0) uniform sampler2D depthTex;

smooth in vec2 texCoord;

out vec4 fragColor;

void main()
{
    float depth = texture(depthTex, texCoord).r;
	fragColor = vec4(depth, depth, depth, 1.0f);
}