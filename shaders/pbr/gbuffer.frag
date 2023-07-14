#version 430

layout (location = 0) out vec4 gPositionAmbientOcclusion;
layout (location = 1) out vec4 gNormalRoughness;
layout (location = 2) out vec4 gBaseColorMetallic;

in vec3 FragPos;
in vec2 TexCoords;
in mat3 TangentToWorldMatrix;

uniform sampler2D texture_base_color;
uniform sampler2D texture_normal;
uniform sampler2D texture_ambient_occlusion;
uniform sampler2D texture_roughness;
uniform sampler2D texture_metallic;

void main()
{
	gPositionAmbientOcclusion.rgb = FragPos;
	gPositionAmbientOcclusion.a  = texture(texture_ambient_occlusion, TexCoords).r;

	vec3 normal = texture(texture_normal, TexCoords).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	gNormalRoughness.rgb = TangentToWorldMatrix * normal;
	gNormalRoughness.a = texture(texture_roughness, TexCoords).r;

	gBaseColorMetallic.rgb = texture(texture_base_color, TexCoords).rgb;
	gBaseColorMetallic.a = texture(texture_metallic, TexCoords).r;
}
