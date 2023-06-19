#version 430

#define MAX_POINT_LIGHTS 8
#define MAX_SPOT_LIGHTS 8

struct Material 
{
    sampler2D texture_normal1;
    sampler2D texture_diffuse1;
    float specular;
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

uniform DirectionalLight directionalLight;

uniform uint pointLightsCount;
uniform PointLight pointLights[MAX_POINT_LIGHTS];

uniform uint spotLightsCount;
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

uniform Material material;

out vec4 FragColor;

//in vec3 FragPos;
in vec2 TexCoords;
in vec3 TangentViewPos;
in vec3 TangentPointLightsPos[MAX_POINT_LIGHTS];
in vec3 TangentFragPos;

vec3 ComputeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection);
vec3 ComputePointLight(PointLight light, vec3 lightPos, vec3 normal, vec3 fragmentPosition, vec3 viewDirection);
vec3 ComputeSpotLight(SpotLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection);

void main()
{
	vec3 normal = texture(material.texture_normal1, TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
    
    vec3 result = vec3(0.0);
    
//    result += ComputeDirectionalLight(directionalLight, normal, viewDir);

    for (uint i = 0u; i < pointLightsCount; i++)
    {
        result += ComputePointLight(pointLights[i], TangentPointLightsPos[i], normal, TangentFragPos, viewDir);
    }

    for (uint i = 0u; i < spotLightsCount; i++)
    {
//        result += ComputeSpotLight(spotLights[i], normal, FragPos, viewDir);
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
    vec3 specular = light.specular * specularIntensity * material.specular;
    return (ambient + diffuse + specular);
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
    vec3 specular = light.specular * specularIntensity * material.specular;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

vec3 ComputeSpotLight(SpotLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection)
{
    vec3 lightDirection = normalize(light.position - fragmentPosition);

    // Diffuse
    float diffuseIntensity = max(dot(normal, lightDirection), 0.0);

    // Specular
    vec3 reflectDirection = reflect(-lightDirection, normal);
    vec3 halfwayDir = normalize(lightDirection + viewDirection);
    float specularIntensity = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

    // Attenuation
    float distance = length(light.position - fragmentPosition);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // Spotlight Intensity
    float theta = dot(lightDirection, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    // Combine
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diffuseIntensity * texture(material.texture_diffuse1, TexCoords).rgb;
    vec3 specular = light.specular * specularIntensity * material.specular;

    ambient *= intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}
