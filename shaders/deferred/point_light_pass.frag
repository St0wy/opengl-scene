#version 430

struct PointLight
{
	vec3 position;

	float linear;
	float quadratic;
	float radius;

	vec3 color;
};

layout (location = 0) out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gBaseColorSpecular;

uniform PointLight pointLight;

uniform vec3 viewPos;
uniform vec2 screenSize;

vec2 CalcTexCoord()
{
	return gl_FragCoord.xy / screenSize;
}

vec3 ComputePointLight(PointLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection, vec3 diffuseTex, float specularTex);

void main()
{
	vec2 texCoord = CalcTexCoord();
	vec3 fragPos = texture(gPosition, texCoord).rgb;
	vec3 normal = texture(gNormal, texCoord).rgb;
	vec3 diffuse = texture(gBaseColorSpecular, texCoord).rgb;
	float specular = texture(gBaseColorSpecular, texCoord).a;

	vec3 viewDir = normalize(viewPos - fragPos);

	vec3 result = ComputePointLight(pointLight, normal, fragPos, viewDir, diffuse, specular);

	FragColor = vec4(result, 1.0);
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
