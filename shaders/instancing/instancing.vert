#version 310 es
precision highp float;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 instanceMatrix;

out vec3 FragPos;
out vec2 TexCoords;

layout (std140) uniform Matrices
{
	mat4 projection;
    mat4 view;
};


void main()
{
    FragPos = vec3(view * instanceMatrix * vec4(aPos, 1.0));
    TexCoords = aTexCoords;

    gl_Position = projection * vec4(FragPos, 1.0);
}
