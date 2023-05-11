#version 310 es
precision highp float;

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 LightPos;
  
uniform vec3 objectColor;
uniform vec3 lightColor;

void main()
{
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(LightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular
    float specularStrength = 0.5;
    float specularPow = 32.0;

    vec3 viewDir = normalize(-FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    vec3 halfwayDir = normalize(lightDir + viewDir);
//    float spec = pow(max(dot(viewDir, reflectDir), 0.0), specularPow);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), specularPow);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
