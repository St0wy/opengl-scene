#version 430

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gBaseColorSpecular;

in vec3 FragPos;
in vec2 TexCoords;
in mat3 TangentToWorldMatrix;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_normal;
uniform sampler2D texture_specular;

void main()
{
	gPosition = FragPos;

	vec3 normal = texture(texture_normal, TexCoords).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	gNormal = TangentToWorldMatrix * normal;

	gBaseColorSpecular.rgb = texture(texture_diffuse, TexCoords).rgb;
	gBaseColorSpecular.a = texture(texture_specular, TexCoords).r;
}
