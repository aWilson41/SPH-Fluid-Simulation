#version 460
layout(binding = 0) uniform sampler2D gPosition;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gDiffuseColor;
layout(binding = 3) uniform sampler2D gAmbientColor;

uniform vec3 lightDir;
uniform vec3 viewDir;
uniform mat4x4 invViewProj;
uniform vec3 eyePos;

smooth in vec2 texCoord;

out vec4 fColor;

// Gets a ray in world space
vec3 getRayDir(vec2 coord)
{
	vec4 clipPos = vec4(texCoord * 2.0f - 1.0f, 0.0f, 1.0f);
	vec4 worldPos = invViewProj * clipPos;

	return normalize(worldPos.xyz / worldPos.w - eyePos);
}

void main()
{
    vec3 pos = texture(gPosition, texCoord).rgb;
    vec3 normal = normalize(texture(gNormal, texCoord).xyz);

	// Warped diffuse (+0.5)
    vec3 diffuse = (max(dot(normal, lightDir), 0.0) * 0.5f + 0.5f) * texture(gDiffuseColor, texCoord).rgb;

	// Ambient light
    vec3 ambient = texture(gAmbientColor, texCoord).rgb;

	// Specular reflection
//	vec3 r = reflect(lightDir, normal);
//	float specular = clamp(pow(dot(r, viewDir), 1.0) * 0.3f, 0.0f, 1.0f);
//
//	// Fresnel
//	float fresnelScale = 0.5f;
//	float fresnelPower = 2.2f;
//	vec3 rayDir = getRayDir(texCoord);
//	float fresnel = min(fresnelScale * pow(1.0f + dot(rayDir, normal), fresnelPower), 1.0);

	if (pos.x != 0.0f || pos.y != 0.0f || pos.z != 0.0f)
		//fColor = vec4(diffuse + specular + fresnel + ambient, 1.0f);
		fColor = vec4(diffuse + ambient, 1.0f);
	else
		fColor = vec4(ambient, 1.0f);

	//fColor = vec4(viewDir.x, viewDir.y, viewDir.z, 1.0f);
	//fColor = vec4(lightDir.x, lightDir.y, lightDir.z, 1.0f);
} 