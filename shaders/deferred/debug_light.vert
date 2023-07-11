#version 430

layout (location = 0) in vec3 aPos;
layout (location = 4) in mat4 modelMatrix;

layout (std140) uniform Matrices
{
	mat4 projection;
	mat4 view;
};

void main()
{
	gl_Position = projection * view * modelMatrix * vec4(aPos, 1.0);
}