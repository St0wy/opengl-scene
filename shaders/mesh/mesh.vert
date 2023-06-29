#version 430

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 4) in mat4 modelMatrix;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

layout (std140, binding = 0) uniform Matrices
{
	mat4 projection;
	mat4 view;
};

void main()
{
	FragPos = vec3(modelMatrix * vec4(aPos, 1.0));
	Normal = mat3(transpose(inverse(modelMatrix))) * aNormal;
	TexCoords = aTexCoords;

	gl_Position = projection * view *  vec4(FragPos, 1.0);
}
