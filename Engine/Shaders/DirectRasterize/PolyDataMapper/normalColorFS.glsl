#version 460

struct PhongMaterial
{
	vec3 diffuseColor;
	vec4 specularColor;
	vec3 ambientColor;
};

uniform PhongMaterial mat;
uniform vec3 lightDir;

smooth in vec3 normal;
smooth in vec3 color;
out vec4 fColor;

void main()
{
	float diffuse = clamp(dot(lightDir, normal), 0.0f, 1.0f);

	vec3 diffuseColor = diffuse * color;
	vec3 ambientColor = mat.ambientColor;

	fColor = vec4(diffuseColor + ambientColor, 1.0f);
}