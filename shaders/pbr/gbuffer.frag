#version 430

layout (location = 0) out vec4 gPositionAmbientOcclusion;
layout (location = 1) out vec4 gNormalRoughness;
layout (location = 2) out vec4 gBaseColorMetallic;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;
in mat3 TangentToWorldMatrix;

uniform sampler2D texture_base_color;
uniform sampler2D texture_normal;
uniform sampler2D texture_ambient_occlusion;
uniform sampler2D texture_roughness;
uniform sampler2D texture_metallic;

void main()
{
    vec2 ddxTexCoord = dFdx(TexCoords);
    vec2 ddyTexCoord = dFdy(TexCoords);
    float ddxLength = length(ddxTexCoord);
    float ddyLength = length(ddyTexCoord);
    float derivativeLength = max(ddxLength, ddyLength);
    float lod = log2(derivativeLength);

    gPositionAmbientOcclusion.rgb = FragPos;
    gPositionAmbientOcclusion.a = textureLod(texture_ambient_occlusion, TexCoords, lod).r;

    vec3 tangentNormal = textureLod(texture_normal, TexCoords, lod).rgb;
    tangentNormal = tangentNormal * 2.0 - 1.0;

    gNormalRoughness.rgb = normalize(TangentToWorldMatrix * tangentNormal);
    gNormalRoughness.a = textureLod(texture_roughness, TexCoords, lod).r;

    gBaseColorMetallic.rgb = textureLod(texture_base_color, TexCoords, lod).rgb;
    gBaseColorMetallic.a = textureLod(texture_metallic, TexCoords, lod).r;
}
