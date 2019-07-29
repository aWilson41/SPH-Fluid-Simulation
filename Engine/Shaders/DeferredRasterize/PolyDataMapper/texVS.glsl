#version 460
uniform mat4 mvp_matrix;

layout (location = 0) in vec3 inPos;
layout (location = 2) in vec2 inTexCoord;

smooth out vec3 pos;
smooth out vec2 texCoord;

void main()
{
	// Calculate vertex position in screen space
	gl_Position = mvp_matrix * vec4(inPos, 1.0);
	pos = vec3(gl_Position);
	texCoord = inTexCoord;
}