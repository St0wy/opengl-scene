#version 430

struct DirectionalLight
{
	vec3 direction;
	vec3 color;
};

layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gBaseColorSpecular;
uniform sampler2D shadowMap;

uniform DirectionalLight directionalLight;

uniform mat4 lightViewProjMatrix;
uniform vec3 viewPos;

vec3 ComputeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection, vec3 fragPos, vec3 diffuseTex, float specularTex);
float ComputeShadowIntensity(vec4 fragPosLightSpace, vec3 normal);

void main()
{
	vec3 fragPos = texture(gPosition, TexCoords).rgb;
	vec3 normal = texture(gNormal, TexCoords).rgb;
	vec3 diffuse = texture(gBaseColorSpecular, TexCoords).rgb;
	float specular = texture(gBaseColorSpecular, TexCoords).a;

	vec3 viewDir = normalize(viewPos - fragPos);

	vec3 result = ComputeDirectionalLight(directionalLight, normal, viewDir, fragPos, diffuse, specular);

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
