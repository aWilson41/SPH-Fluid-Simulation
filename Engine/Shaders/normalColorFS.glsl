#version 460
uniform vec3 lightDir;

smooth in vec3 inFNormal;
smooth in vec3 inFColor;

out vec4 fColor;

void main()
{
	vec3 color = clamp(dot(lightDir, inFNormal) * inFColor, 0.0f, 1.0f);
	fColor = vec4(color, 1.0f);
}