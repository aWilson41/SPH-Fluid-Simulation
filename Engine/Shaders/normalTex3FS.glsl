#version 460
uniform sampler2D tex;
uniform vec3 lightDir;

smooth in vec3 inFNormal;
smooth in vec2 inFTexCoord;

out vec4 fColor;

void main()
{
	vec3 color = clamp(dot(lightDir, inFNormal) * vec3(texture2D(tex, inFTexCoord)), 0.0f, 1.0f);
	fColor = vec4(color, 1.0f);
}