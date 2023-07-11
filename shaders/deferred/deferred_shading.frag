#version 430

#define MAX_POINT_LIGHTS 32
#define MAX_SPOT_LIGHTS 1

struct DirectionalLight
{
	vec3 direction;

	vec3 color;
};

struct PointLight
{
	vec3 position;

	float linear;
	float quadratic;
	float radius;

	vec3 color;
};

struct SpotLight
{
	vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;

	float linear;
	float quadratic;
	float radius;

	vec3 color;
};

layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gBaseColorSpecular;
uniform sampler2D shadowMap;

uniform DirectionalLight directionalLight;

uniform uint pointLightsCount;
uniform PointLight pointLights[MAX_POINT_LIGHTS];

uniform uint spotLightsCount;
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

uniform vec3 viewPos;

uniform mat4 lightViewProjMatrix;

vec3 ComputeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection, vec3 fragPos, vec3 diffuseTex, float specularTex);
vec3 ComputePointLight(PointLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection, vec3 diffuseTex, float specularTex);
vec3 ComputeSpotLight(SpotLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection, vec3 diffuseTex, float specularTex);
float ComputeShadowIntensity(vec4 fragPosLightSpace, vec3 normal);

void main()
{
	vec3 fragPos = texture(gPosition, TexCoords).rgb;
	vec3 normal = texture(gNormal, TexCoords).rgb;
	vec3 diffuse = texture(gBaseColorSpecular, TexCoords).rgb;
	float specular = texture(gBaseColorSpecular, TexCoords).a;

	vec3 viewDir = normalize(viewPos - fragPos);

	vec3 result = vec3(0.0);

	result += ComputeDirectionalLight(directionalLight, normal, viewDir, fragPos, diffuse, specular);

	for (uint i = 0u; i < pointLightsCount; i++)
	{
		float distance = length(pointLights[i].position - fragPos);
		if (distance >= pointLights[i].radius)
		{
			continue;
		}
		result += ComputePointLight(pointLights[i], normal, fragPos, viewDir, diffuse, specular);
	}

	for (uint i = 0u; i < spotLightsCount; i++)
	{
		float distance = length(spotLights[i].position - fragPos);
		if (distance >= spotLights[i].radius)
		{
			continue;
		}
		result += ComputeSpotLight(spotLights[i], normal, fragPos, viewDir, diffuse, specular);
	}

	FragColor = vec4(result, 1.0);
}

vec3 ComputeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection, vec3 fragPos, vec3 diffuseTex, float specularTex)
{
	vec3 lightDirection = normalize(-light.direction);

	// Diffuse
	float diffuseIntensity = max(dot(normal, lightDirection), 0.0);

	// Specular
	vec3 reflectDirection = reflect(-lightDirection, normal);
	vec3 halfwayDir = normalize(lightDirection + viewDirection);
	float specularIntensity = pow(max(dot(normal, halfwayDir), 0.0), 16.0);

	vec3 diffuse = light.color * diffuseIntensity * diffuseTex;
	vec3 specular = light.color * specularIntensity * specularTex;

	float shadow = ComputeShadowIntensity(lightViewProjMatrix * vec4(fragPos, 1.0), normal);
	return (1.0 - shadow) * (diffuse + specular);
}

vec3 ComputePointLight(PointLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection, vec3 diffuseTex, float specularTex)
{
	vec3 lightDirection = normalize(light.position - fragmentPosition);

	// Diffuse
	float diffuseIntensity = max(dot(normal, lightDirection), 0.0);

	// Specular
	vec3 reflectDirection = reflect(-lightDirection, normal);
	vec3 halfwayDir = normalize(lightDirection + viewDirection);
	float specularIntensity = pow(max(dot(normal, halfwayDir), 0.0), 16.0);

	// Attenuation
	float distance = length(light.position - fragmentPosition);
	float attenuation = 1.0 / (1.0 + light.linear * distance + light.quadratic * (distance * distance));

	vec3 diffuse = light.color * diffuseIntensity * diffuseTex;
	vec3 specular = light.color * specularIntensity * specularTex;

	diffuse *= attenuation;
	specular *= attenuation;

	return (diffuse + specular);
}

vec3 ComputeSpotLight(SpotLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection, vec3 diffuseTex, float specularTex)
{
	vec3 lightDirection = normalize(light.position - fragmentPosition);

	// Diffuse
	float diffuseIntensity = max(dot(normal, lightDirection), 0.0);

	// Specular
	vec3 reflectDirection = reflect(-lightDirection, normal);
	vec3 halfwayDir = normalize(lightDirection + viewDirection);
	float specularIntensity = pow(max(dot(normal, halfwayDir), 0.0), 16.0);

	// Attenuation
	float distance = length(light.position - fragmentPosition);
	float attenuation = 1.0 / (light.linear * distance + light.quadratic * (distance * distance));

	// Spotlight Intensity
	float theta = dot(lightDirection, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	// Combine
	vec3 diffuse = light.color * diffuseIntensity * diffuseTex;
	vec3 specular = light.color * specularIntensity * specularTex;

	diffuse *= attenuation * intensity;
	specular *= attenuation * intensity;

	return (diffuse + specular);
}

float ComputeShadowIntensity(vec4 fragPosLightSpace, vec3 normal)
{
	// Perform perspective divide
	vec3 projectionCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	// Transform to [0,1] range
	projectionCoords = projectionCoords * 0.5 + 0.5;

	// Get closest depth value from light's perspective (using [0,1] range fragPosLight as coordinates)
	float closestDepth = texture(shadowMap, projectionCoords.xy).r;

	// Get depth of current fragment from light's perspective
	float currentDepth = projectionCoords.z;

	// Compute bias (based on depth map resolution and slope)
	vec3 lightDirection = normalize(directionalLight.direction);
	float bias = max(0.05 * (1.0 / dot(normal, lightDirection)), 0.005);

	// Check whether current frag pos is in shadow
	//	float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
	// PCF (Percentage-Closer Filtering)
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMap, projectionCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;

	// Keep the shadow at 0.0 when outside the far_plane region of the light's frustum
	if (projectionCoords.z > 1.0)
	{
		shadow = 0.0;
		FragColor = vec4(1.0);
	}

	//	return closestDepth;
	return shadow;
}