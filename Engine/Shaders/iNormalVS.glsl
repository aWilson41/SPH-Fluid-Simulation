#version 460
uniform mat4 mvp_matrix;

layout(location = 0) in vec3 inVPos;
layout(location = 1) in vec3 inVNormal;
layout(location = 4) in vec3 inVOffset;

smooth out vec3 inFNormal;

void main()
{
	// Calculate vertex position in screen space
	gl_Position = mvp_matrix * vec4(inVPos + inVOffset, 1.0);
	inFNormal = inVNormal;
}