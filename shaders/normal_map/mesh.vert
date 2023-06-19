#version 430

#define MAX_POINT_LIGHTS 8
#define MAX_SPOT_LIGHTS 8

struct Material 
{
    sampler2D texture_diffuse1;
    float specular;
    float shininess;
    sampler2D texture_normal1;
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

uniform vec3 viewPos;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in mat4 modelMatrix;

//out vec3 FragPos;
//out vec3 Normal;
out vec2 TexCoords;
out vec3 TangentViewPos;
out vec3 TangentPointLightsPos[MAX_POINT_LIGHTS];
out vec3 TangentFragPos;

layout (std140) uniform Matrices
{
	mat4 projection;
    mat4 view;
};

void main()
{

    TexCoords = aTexCoords;
    vec3 fragPos = vec3(modelMatrix * vec4(aPos, 1.0));

    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));

    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);

    for (int i = 0; i < pointLightsCount; i++)
    {
        TangentPointLightsPos[i] = TBN * pointLights[i].position;
    }
    TangentViewPos = TBN * viewPos;
    TangentFragPos = TBN * fragPos;

    gl_Position = projection * view * modelMatrix * vec4(aPos, 1.0);
}