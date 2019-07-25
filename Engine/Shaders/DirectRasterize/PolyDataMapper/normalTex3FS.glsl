#version 460

struct PhongMaterial
{
	vec3 diffuseColor;
	vec4 specularColor;
	vec3 ambientColor;
};

uniform PhongMaterial mat;
uniform sampler2D tex;
uniform vec3 lightDir;

smooth in vec3 normal;
smooth in vec2 texCoord;
out vec4 fColor;

void main()
{
	float diffuse = clamp(dot(lightDir, normal), 0.0f, 1.0f);

	vec3 diffuseColor = diffuse * vec3(texture2D(tex, texCoord);
	vec3 ambientColor = mat.ambientColor;

	fColor = vec4(diffuseColor + ambientColor, 1.0f);
}