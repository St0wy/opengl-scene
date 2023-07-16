#version 430

const float PI = 3.14159265359;
const float MAX_REFLECTION_LOD = 4.0;

layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPositionAmbientOcclusion;
uniform sampler2D gNormalRoughness;
uniform sampler2D gBaseColorMetallic;
uniform sampler2D gSsao;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLut;

uniform vec3 viewPos;

vec3 FresnelSchlick(float cosTheta, vec3 F0);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

void main()
{
	vec3 fragPos = texture(gPositionAmbientOcclusion, TexCoords).rgb;
	vec3 normal = texture(gNormalRoughness, TexCoords).rgb;
	vec3 baseColor = texture(gBaseColorMetallic, TexCoords).rgb;
	float roughness = texture(gNormalRoughness, TexCoords).a;
	float metallic = texture(gBaseColorMetallic, TexCoords).a;
	float ambientOcclusion = texture(gSsao, TexCoords).r * texture(gPositionAmbientOcclusion, TexCoords).a;

	// V
	vec3 viewDir = normalize(viewPos - fragPos);
	// R
	vec3 reflection = reflect(viewDir, normal);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, baseColor, metallic);

	vec3 kSpecular = FresnelSchlickRoughness(max(dot(normal, viewDir), 0.0), F0, roughness);
	vec3 kDiffuse = 1.0 - kSpecular;
	kDiffuse *= 1.0 - metallic;

	vec3 irradiance = texture(irradianceMap, normal).rgb;
	vec3 diffuse = irradiance * baseColor;

	// sample both the pre-filter map and the BRDF lut and combine them together
	// as per the Split-Sum approximation to get the IBL specular part
	vec3 prefilteredColor = textureLod(prefilterMap, reflection, roughness * MAX_REFLECTION_LOD).rgb;
	vec2 brdf = texture(brdfLut, vec2(max(dot(normal, viewDir), 0.0), roughness)).rg;
	vec3 specular = prefilteredColor * (kSpecular * brdf.x + brdf.y);

	vec3 ambient = (kDiffuse * diffuse + specular) * ambientOcclusion;

//	ambient = vec3(kSpecular);
	FragColor = vec4(ambient, 1.0);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a      = roughness*roughness;
	float a2     = a*a;
	float NdotH  = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;

	float num   = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

	float num   = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2  = GeometrySchlickGGX(NdotV, roughness);
	float ggx1  = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
