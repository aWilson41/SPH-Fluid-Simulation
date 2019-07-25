#version 460

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform vec3 lightDir;

in vec2 texCoord;

out vec4 fColor;

void main()
{
    vec3 pos = texture(gPosition, texCoord).rgb;
    vec3 normal = texture(gNormal, texCoord).rgb;
    vec3 color = texture(gAlbedoSpec, texCoord).rgb;
    //float Specular = texture(gAlbedoSpec, texCoord).a;
    
	fColor = vec4(max(dot(normal, lightDir), 0.0) * color, 1.0f);
} 