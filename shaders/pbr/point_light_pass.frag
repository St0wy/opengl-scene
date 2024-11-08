#version 430

const float PI = 3.14159265359;
const float MAX_REFLECTION_LOD = 4.0;

struct PointLight
{
    vec3 position;
    vec3 color;
};

layout (location = 0) out vec4 FragColor;

uniform sampler2D gPositionAmbientOcclusion;
uniform sampler2D gNormalRoughness;
uniform sampler2D gBaseColorMetallic;

uniform PointLight pointLight;

uniform vec2 screenSize;
uniform vec3 viewPos;

vec2 CalcTexCoord()
{
    return gl_FragCoord.xy / screenSize;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

void main()
{
    vec2 texCoord = CalcTexCoord();
    vec3 fragPos = textureLod(gPositionAmbientOcclusion, texCoord, 0).rgb;
    vec3 normal = textureLod(gNormalRoughness, texCoord, 0).rgb;
    vec3 baseColor = textureLod(gBaseColorMetallic, texCoord, 0).rgb;
    float roughness = textureLod(gNormalRoughness, texCoord, 0).a;
    float metallic = textureLod(gBaseColorMetallic, texCoord, 0).a;

    // V
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, baseColor, metallic);

    vec3 Lo = vec3(0.0);

    // L
    vec3 fragToLightDir = normalize(pointLight.position - fragPos);
    // H
    vec3 halfwayDir = normalize(fragToLightDir + viewDir);

    float distance = distance(pointLight.position, fragPos);

    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = pointLight.color * attenuation;

    // F
    vec3 fresnel = FresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);
    float normalDistributionFunction = DistributionGGX(normal, halfwayDir, roughness);
    float geometry = GeometrySmith(normal, viewDir, fragToLightDir, roughness);

    vec3 kSpecular = fresnel;
    vec3 kDiffuse = vec3(1.0) - kSpecular;
    kDiffuse *= 1.0 - metallic;

    // Cook-Torrance BRDF
    vec3 numerator = normalDistributionFunction * geometry * fresnel;
    // We add a small number to prevent division by 0
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, fragToLightDir), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    float normalDotFragToLightDir = max(dot(normal, fragToLightDir), 0.0);

    vec3 color = (kDiffuse * baseColor / PI + specular) * radiance * normalDotFragToLightDir;

    FragColor = vec4(color, 1.0);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
