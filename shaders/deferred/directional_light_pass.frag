#version 430

struct DirectionalLight
{
	vec3 direction;
	vec3 color;
};

const int NUM_CASCADES = 4;

layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gBaseColorSpecular;
uniform sampler2D shadowMaps[NUM_CASCADES];

uniform DirectionalLight directionalLight;

uniform mat4 lightViewProjMatrix[NUM_CASCADES];
uniform mat4 viewMatrix;
uniform vec3 viewPos;
uniform vec4 csmFarDistances;

vec3 ComputeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection, vec3 fragPos, vec3 diffuseTex, float specularTex);
float ComputeShadowIntensity(int cascadeIndex, vec4 fragPosLightSpace, vec3 normal);

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

	vec4 viewFragPos = viewMatrix * vec4(fragPos, 1.0);
	float depthValue = -viewFragPos.z;
	vec4 res = step(csmFarDistances, vec4(depthValue));
	int shadowCascadeIndex = int(res.x + res.y + res.z + res.w);
	float shadow = ComputeShadowIntensity(shadowCascadeIndex, lightViewProjMatrix[shadowCascadeIndex] * vec4(fragPos, 1.0), normal);

	vec3 hint = vec3(0.0);
	if (shadowCascadeIndex == 0){
		hint = vec3(0.05, 0.0, 0.0);
	} else if (shadowCascadeIndex == 1){
		hint = vec3(0.0, 0.05, 0.0);
	} else if (shadowCascadeIndex == 2){
		hint = vec3(0.0, 0.0, 0.05);
	} else if (shadowCascadeIndex == 3){
		hint = vec3(0.05, 0.05, 0.0);
	} else {
		hint = vec3(1.0, 1.0, 1.0);
	}

	//		return (1.0 - shadow) * (diffuse + specular) + hint;
	return (1.0 - shadow) * (diffuse + specular);
	//	return vec3(depthValue);
}

float ComputeShadowIntensity(int cascadeIndex, vec4 fragPosLightSpace, vec3 normal)
{
	// Perform perspective divide
	vec3 projectionCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	// Transform to [0,1] range
	projectionCoords = projectionCoords * 0.5 + 0.5;

	// Get closest depth value from light's perspective (using [0,1] range fragPosLight as coordinates)
	float closestDepth = texture(shadowMaps[cascadeIndex], projectionCoords.xy).r;

	// Get depth of current fragment from light's perspective
	float currentDepth = projectionCoords.z;

	// Compute bias (based on depth map resolution and slope)
	vec3 lightDirection = normalize(directionalLight.direction);
	float bias = max(0.05 * (1.0 / dot(normal, lightDirection)), 0.005);

	// Check whether current frag pos is in shadow
	//		float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
	// PCF (Percentage-Closer Filtering)
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMaps[cascadeIndex], 0);
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMaps[cascadeIndex], projectionCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;

	// Keep the shadow at 0.0 when outside the far_plane region of the light's frustum
	if (projectionCoords.z > 1.0)
	{
		shadow = 0.0;
	}

	//	return closestDepth;
	return shadow;
}
