#version 460
uniform sampler2D tex;

smooth in vec2 inFTexCoord;

out vec4 fColor;

void main()
{
	fColor = texture2D(tex, inFTexCoord);
}