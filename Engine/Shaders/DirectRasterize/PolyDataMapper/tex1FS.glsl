#version 460

uniform sampler2D tex;

smooth in vec2 texCoord;
out vec4 fColor;

void main()
{
	vec3 ambientColor = vec3(texture2D(tex, texCoord));

	fColor = vec4(ambientColor.x, ambientColor.x, ambientColor.x, 1.0f);
}