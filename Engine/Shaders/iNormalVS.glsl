#version 460
uniform mat4 mvp_matrix;

in vec3 inPos;
in vec3 inNormal;
in vec3 inOffset;

smooth out vec3 normal;

void main()
{
	// Calculate vertex position in screen space
	gl_Position = mvp_matrix * vec4(inPos + inOffset, 1.0);
	normal = inNormal;
}