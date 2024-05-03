#version 430
// This is needed for CSM : https://gist.github.com/JuanDiegoMontoya/55482fc04d70e83729bb9528ecdc1c61
#extension GL_NV_gpu_shader5: enable
#extension GL_EXT_nonuniform_qualifier: enable

#ifdef GL_EXT_nonuniform_qualifier
    #define NonUniformIndex(x) nonuniformEXT(x)
#else
    #define NonUniformIndex(x) (x)
#endif

struct DirectionalLight
{
    vec3 direction;
    vec3 color;
};

const int NUM_CASCADES = 4;
const float PI = 3.14159265359;

layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPositionAmbientOcclusion;
uniform sampler2D gNormalRoughness;
uniform sampler2D gBaseColorMetallic;
uniform sampler2D shadowMaps[NUM_CASCADES];

uniform DirectionalLight directionalLight;

uniform mat4 lightViewProjMatrix[NUM_CASCADES];
uniform vec3 viewPos;
uniform vec4 csmFarDistances;

layout (std140, binding = 0) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

float ComputeShadowIntensity(int cascadeIndex, vec4 fragPosLightSpace, vec3 normal);
vec3 FresnelSchlick(float cosTheta, vec3 F0);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

void main()
{
    vec3 fragPos = textureLod(gPositionAmbientOcclusion, TexCoords, 0).rgb;
    vec3 normal = textureLod(gNormalRoughness, TexCoords, 0).rgb;
    vec3 baseColor = textureLod(gBaseColorMetallic, TexCoords, 0).rgb;
    float roughness = textureLod(gNormalRoughness, TexCoords, 0).a;
    float metallic = textureLod(gBaseColorMetallic, TexCoords, 0).a;

    vec3 viewDir = normalize(-fragPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, baseColor, metallic);

    // L
    vec3 fragToLightDir = normalize(-directionalLight.direction);
    // H
    vec3 halfwayDir = normalize(fragToLightDir + viewDir);

    vec3 radiance = directionalLight.color;

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

    vec4 viewFragPos = view * vec4(fragPos, 1.0);
    float depthValue = -viewFragPos.z;
    vec4 res = step(csmFarDistances, vec4(depthValue));
    int shadowCascadeIndex = int(res.x + res.y + res.z + res.w);

    vec4 fragPosLightSpace = lightViewProjMatrix[shadowCascadeIndex] * vec4(fragPos, 1.0);
    float shadow = ComputeShadowIntensity(shadowCascadeIndex, fragPosLightSpace, normal);

    color *= (1.0 - shadow);

    //    vec3 hint = vec3(0.0);
    //    if (shadowCascadeIndex == 0) {
    //        hint = vec3(0.05, 0.0, 0.0);
    //    } else if (shadowCascadeIndex == 1) {
    //        hint = vec3(0.0, 0.05, 0.0);
    //    } else if (shadowCascadeIndex == 2) {
    //        hint = vec3(0.0, 0.0, 0.05);
    //    } else if (shadowCascadeIndex == 3) {
    //        hint = vec3(0.05, 0.05, 0.0);
    //    } else {
    //        hint = vec3(1.0, 1.0, 1.0);
    //    }
    //    color += hint;

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

float ComputeShadowIntensity(int cascadeIndex, vec4 fragPosLightSpace, vec3 normal)
{
    // Perform perspective divide
    vec3 projectionCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // Transform to [0,1] range
    projectionCoords = projectionCoords * 0.5 + 0.5;

    // Keep the shadow at 0.0 when outside the far_plane region of the light's frustum
    if (projectionCoords.z > 1.0)
    {
        return 0.0;
    }

    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coordinates)
    float closestDepth = textureLod(shadowMaps[NonUniformIndex(cascadeIndex)], projectionCoords.xy, 0).r;

    // Get depth of current fragment from light's perspective
    float currentDepth = projectionCoords.z;

    // Compute bias (based on depth map resolution and slope)
    vec3 lightDirection = normalize(directionalLight.direction);
    float bias = max(0.0005 * (1.0 / dot(normal, lightDirection)), 0.0002);

    // PCF (Percentage-Closer Filtering)
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMaps[NonUniformIndex(cascadeIndex)], 0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = textureLod(shadowMaps[NonUniformIndex(cascadeIndex)], projectionCoords.xy + vec2(x, y) * texelSize, 0).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
            // shadow += currentDepth > pcfDepth ? 1.0 : 0.0;
        }
    }

    return shadow / 9.0;
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
