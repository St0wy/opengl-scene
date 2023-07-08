#version 430

#define MAX_POINT_LIGHTS 8
#define MAX_SPOT_LIGHTS 8

struct Material
{
	sampler2D texture_normal1;
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	float shininess;
};


struct DirectionalLight
{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight
{
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct SpotLight
{
	vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

out vec4 FragColor;

in vec2 TexCoords;
in vec4 FragPosLightSpace;

in vec3 TangentViewPos;
in vec3 TangentPointLightsPos[MAX_POINT_LIGHTS];
in vec3 TangentSpotLightsPos[MAX_SPOT_LIGHTS];
in vec3 TangentSpotLightsDir[MAX_SPOT_LIGHTS];
in vec3 TangentFragPos;

uniform DirectionalLight directionalLight;

uniform uint pointLightsCount;
uniform PointLight pointLights[MAX_POINT_LIGHTS];

uniform uint spotLightsCount;
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

uniform Material material;
uniform vec3 viewPos;

uniform sampler2D shadowMap;

vec3 ComputeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection);
vec3 ComputePointLight(PointLight light, vec3 lightPos, vec3 normal, vec3 fragmentPosition, vec3 viewDirection);
vec3 ComputeSpotLight(SpotLight light, vec3 lightPos, vec3 lightDir, vec3 normal, vec3 fragmentPosition, vec3 viewDirection);
float ComputeShadowIntensity(vec4 fragPosLightSpace, vec3 normal);

void main()
{
	vec3 normal = texture(material.texture_normal1, TexCoords).rgb;
	normal = normalize(normal * 2.0 - 1.0);

	vec3 viewDir = normalize(TangentViewPos - TangentFragPos);

	vec3 result = vec3(0.0);

	float alpha = texture(material.texture_diffuse1, TexCoords).a;

	if (alpha < 0.1)
	{
		discard;
	}

	result += ComputeDirectionalLight(directionalLight, normal, viewDir);

	for (uint i = 0u; i < pointLightsCount; i++)
	{
		result += ComputePointLight(pointLights[i], TangentPointLightsPos[i], normal, TangentFragPos, viewDir);
	}

	for (uint i = 0u; i < spotLightsCount; i++)
	{
		result += ComputeSpotLight(spotLights[i], TangentSpotLightsPos[i], TangentSpotLightsDir[i], normal, TangentFragPos, viewDir);
	}

	float gamma = 2.2;
	result = pow(result, vec3(1.0 / gamma));
	FragColor = vec4(result, 1.0);
}

vec3 ComputeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection)
{
	vec3 lightDirection = normalize(-light.direction);

	// Diffuse
	float diffuseIntensity = max(dot(normal, lightDirection), 0.0);

	// Specular
	vec3 reflectDirection = reflect(-lightDirection, normal);
	vec3 halfwayDir = normalize(lightDirection + viewDirection);
	float specularIntensity = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

	vec3 ambient = light.ambient * texture(material.texture_diffuse1, TexCoords).rgb;
	vec3 diffuse = light.diffuse * diffuseIntensity * texture(material.texture_diffuse1, TexCoords).rgb;
	vec3 specular = light.specular * specularIntensity * texture(material.texture_specular1, TexCoords).rgb;

	float shadow = ComputeShadowIntensity(FragPosLightSpace, normal);
	return (ambient + (1.0 - shadow) * (diffuse + specular));
}

vec3 ComputePointLight(PointLight light, vec3 lightPos, vec3 normal, vec3 fragmentPosition, vec3 viewDirection)
{
	vec3 lightDirection = normalize(lightPos - fragmentPosition);

	// Diffuse
	float diffuseIntensity = max(dot(normal, lightDirection), 0.0);

	// Specular
	vec3 reflectDirection = reflect(-lightDirection, normal);
	vec3 halfwayDir = normalize(lightDirection + viewDirection);
	float specularIntensity = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

	// Attenuation
	float distance = length(lightPos - fragmentPosition);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	vec3 ambient = light.ambient * texture(material.texture_diffuse1, TexCoords).rgb;
	vec3 diffuse = light.diffuse * diffuseIntensity * texture(material.texture_diffuse1, TexCoords).rgb;
	vec3 specular = light.specular * specularIntensity * texture(material.texture_specular1, TexCoords).rgb;

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}

vec3 ComputeSpotLight(SpotLight light, vec3 lightPos, vec3 lightDir, vec3 normal, vec3 fragmentPosition, vec3 viewDirection)
{
	vec3 lightDirection = normalize(lightPos - fragmentPosition);

	// Diffuse
	float diffuseIntensity = max(dot(normal, lightDirection), 0.0);

	// Specular
	vec3 reflectDirection = reflect(-lightDirection, normal);
	vec3 halfwayDir = normalize(lightDirection + viewDirection);
	float specularIntensity = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

	// Attenuation
	float distance = length(lightPos - fragmentPosition);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	// Spotlight Intensity
	float theta = dot(lightDirection, normalize(-lightDir));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	// Combine
	vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
	vec3 diffuse = light.diffuse * diffuseIntensity * texture(material.texture_diffuse1, TexCoords).rgb;
	vec3 specular = light.specular * specularIntensity * texture(material.texture_specular1, TexCoords).rgb;

	ambient *= intensity;
	diffuse *= attenuation * intensity;
	specular *= attenuation * intensity;

	return (ambient + diffuse + specular);
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