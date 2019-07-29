#version 460
uniform sampler2D tex;

smooth in vec3 pos;
smooth in vec2 texCoord;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gDiffuseColor;
layout (location = 3) out vec4 gAmbientColor;

void main()
{
	gPosition = pos;
	gNormal = vec3(0.0f, 0.0f, 1.0f);
	gDiffuseColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	gAmbientColor = texture2D(tex, texCoord);
}