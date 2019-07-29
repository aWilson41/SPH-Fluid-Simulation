#version 460
const vec2 quadVertices[4] = { vec2(-1.0f, -1.0f), vec2(1.0f, -1.0f), vec2(-1.0f, 1.0f), vec2(1.0f, 1.0f) };
const vec2 quadTexCoords[4] = { vec2(0.0f, 0.0f), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f), vec2(1.0f, 1.0f) };

smooth out vec2 texCoord;

void main()
{
    // Calculate vertex position in screen space
    gl_Position = vec4(quadVertices[gl_VertexID], 0.0f, 1.0f);
	texCoord = quadTexCoords[gl_VertexID];
}