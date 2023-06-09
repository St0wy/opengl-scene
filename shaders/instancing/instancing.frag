#version 310 es
precision highp float;

#define MAX_POINT_LIGHTS 32
#define MAX_DIRECTIONAL_LIGHTS 8
#define MAX_SPOT_LIGHTS 32

struct Material 
{
    sampler2D texture_diffuse1;
};

out vec4 FragColor;

in vec3 FragPos;
//in vec3 Normal;
in vec2 TexCoords;

uniform Material material;

void main()
{
//	vec3 norm = normalize(Normal);
    vec3 result = texture(material.texture_diffuse1, TexCoords).rgb;

    FragColor = vec4(result, 1.0);
}
