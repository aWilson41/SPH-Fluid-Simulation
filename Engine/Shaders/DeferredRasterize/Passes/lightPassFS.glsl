#version 460
layout(binding = 0) uniform sampler2D gPosition;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gDiffuseColor;
layout(binding = 3) uniform sampler2D gAmbientColor;

uniform vec3 lightDir;
uniform vec3 viewDir;
uniform mat4x4 invProj;

smooth in vec2 texCoord;

out vec4 fColor;

vec3 getRayDir(vec2 coord)
{
	vec4 clipPos = vec4(texCoord * 2.0f - 1.0f, 0.0f, 1.0f);
	vec4 viewPos = invProj * clipPos;

	return normalize(viewPos.xyz / viewPos.w);
}

void main()
{
    vec3 pos = texture(gPosition, texCoord).rgb;
    vec3 normal = normalize(texture(gNormal, texCoord).rgb);

    vec3 diffuse = (max(dot(normal, lightDir), 0.0) * 0.5f + 0.5f) * texture(gDiffuseColor, texCoord).rgb;
    vec3 ambient = texture(gAmbientColor, texCoord).rgb;
//    float specular = 0.0f;
//	if (dot(normal, lightDir) > 0.0f)
//		specular = pow(clamp(dot(reflect(lightDir, normal), viewDir), 0.0f, 1.0f), 3.0) * 0.5f;

	float fresnelScale = 0.5f;
	float fresnelPower = 2.2f;
	vec3 rayDir = getRayDir(texCoord);
	float fresnel = min(fresnelScale * pow(1.0f + dot(rayDir, normal), fresnelPower), 1.0);

	if (pos.x != 0.0f || pos.y != 0.0f || pos.z != 0.0f)
		fColor = vec4(diffuse + fresnel + ambient, 1.0f);
		//fColor = vec4(diffuse + fresnel + specular + ambient, 1.0f);
	else
		fColor = vec4(ambient, 1.0f);
} 