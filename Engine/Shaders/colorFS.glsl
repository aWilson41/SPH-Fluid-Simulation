#version 460

smooth in vec3 inFColor;

out vec4 fColor;

void main()
{
	fColor = vec4(inFColor, 1.0);
}