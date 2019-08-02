#version 460
layout(binding = 0) uniform sampler2D colorTex;

smooth in vec2 texCoord;

out vec4 fColor;

void main()
{
    fColor = texture(colorTex, texCoord).rgba;
} 