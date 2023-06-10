#version 310 es
precision highp float;

struct Material 
{
	sampler2D texture_diffuse1;
};

#shader vertex
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec2 TexCoords;

layout (std140) uniform Matrices
{
	mat4 projection;
    mat4 view;
};

uniform mat4 model;

void main()
{
    FragPos = vec3(view * model * vec4(aPos, 1.0));
    TexCoords = aTexCoords;

    gl_Position = projection * vec4(FragPos, 1.0);
}

#shader fragment

out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;

uniform Material material;

void main()
{
    vec3 result = texture(material.texture_diffuse1, TexCoords).rgb;

    FragColor = vec4(result, 1.0);
}
