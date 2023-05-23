#version 310 es
precision highp float;

#define MAX_POINT_LIGHT 4

struct Material {
    sampler2D diffuse;
    sampler2D specular;
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

struct SpotLight {
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

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

//uniform Light light;
uniform vec3 viewPos;
uniform DirectionalLight directionalLight;
uniform PointLight pointLights[MAX_POINT_LIGHT];
uniform SpotLight spotLight;

uniform Material material;

vec3 ComputeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection);
vec3 ComputePointLight(PointLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection);
vec3 ComputeSpotLight(SpotLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection);

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(-FragPos);

    vec3 result = ComputeDirectionalLight(directionalLight, norm, viewDir);
    for (int i = 0; i < MAX_POINT_LIGHT; i++)
    {
        result += ComputePointLight(pointLights[i], norm, FragPos, viewDir);
    }

    result += ComputeSpotLight(spotLight, norm, FragPos, viewDir);

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

    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;
    vec3 diffuse = light.diffuse * diffuseIntensity * texture(material.diffuse, TexCoords).rgb;
    vec3 specular = light.specular * specularIntensity * texture(material.specular, TexCoords).rgb;
    return (ambient + diffuse + specular);
}

vec3 ComputePointLight(PointLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection)
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

    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;
    vec3 diffuse = light.diffuse * diffuseIntensity * texture(material.diffuse, TexCoords).rgb;
    vec3 specular = light.specular * specularIntensity * texture(material.specular, TexCoords).rgb;

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
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diffuseIntensity * texture(material.diffuse, TexCoords).rgb;
    vec3 specular = light.specular * specularIntensity * texture(material.specular, TexCoords).rgb;

    ambient *= intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}
