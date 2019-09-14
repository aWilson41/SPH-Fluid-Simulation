// Computes the normals in world space given a depth image
#version 460

layout(binding = 0) uniform sampler2D inputTex;

uniform float maxDepth;
uniform mat4x4 invProj;
uniform mat4x4 invView;
uniform float nearZ;
uniform float farZ;
uniform vec2 texelSize;

smooth in vec2 texCoord;

out vec3 fragNormal;

float unlinearizeDepth(float z)
{
	return -(2.0f * nearZ * farZ / z - farZ - nearZ) / (farZ - nearZ);
}

vec3 getEyePos(vec2 texCoord)
{
	float z = unlinearizeDepth(texture(inputTex, texCoord).r);
	vec4 clipPos = vec4(vec3(texCoord, z) * 2.0f - 1.0f, 1.0f);
	vec4 viewPosProj = invProj * clipPos;
	vec4 viewPos = viewPosProj / viewPosProj.w;
	return vec3(invView * viewPos);
}

void main()
{
    // read eye-space depth from texture
	float depth = texture(inputTex, texCoord).r;
	if (depth > maxDepth)
		discard;

	// Calculate the position of this fragment (view space)
	vec3 pos = getEyePos(texCoord);

	// Compute surface tangents
	// Compute left and right depth
	vec3 ddx = getEyePos(texCoord + vec2(texelSize.x, 0)) - pos;
	vec3 ddx2 = pos - getEyePos(texCoord - vec2(texelSize.x, 0));
	// Use the smaller one as it's closest
	if (abs(ddx2.z) < abs(ddx.z))
		ddx = ddx2;

	vec3 ddy = getEyePos(texCoord + vec2(0, texelSize.y)) - pos;
	vec3 ddy2 = pos - getEyePos(texCoord - vec2(0, texelSize.y));
	if (abs(ddy2.z) < abs(ddy.z))
		ddy = ddy2;

	// Calculate normal
	vec3 n = cross(ddx, ddy);
	fragNormal = normalize(n);
}