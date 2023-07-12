#version 430

layout (location = 0) in vec3 aPos;

layout (std140) uniform Matrices
{
	mat4 projection;
	mat4 view;
};

uniform mat4 modelMatrix;

void main()
{
	gl_Position = projection * view * modelMatrix * vec4(aPos, 1.0);
}
