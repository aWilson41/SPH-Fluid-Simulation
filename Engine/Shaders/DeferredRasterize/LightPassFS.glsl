#version 460
layout(binding = 0) uniform sampler2D gPosition;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gDiffuseColor;
layout(binding = 3) uniform sampler2D gAmbientColor;
uniform vec3 lightDir;

smooth in vec2 texCoord;

out vec4 fColor;

void main()
{
    vec3 pos = texture(gPosition, texCoord).rgb;
    vec3 normal = texture(gNormal, texCoord).rgb;
    vec3 diffuseColor = texture(gDiffuseColor, texCoord).rgb;
    vec3 ambientColor = texture(gAmbientColor, texCoord).rgb;
    //float Specular = texture(gAlbedoSpec, texCoord).a;
    
	fColor = vec4(max(dot(normal, lightDir), 0.0) * diffuseColor + ambientColor, 1.0f);
} 