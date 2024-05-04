#version 430

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in mat4 modelMatrix;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out mat3 TangentToWorldMatrix;

layout (std140, binding = 0) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

void main()
{
    TexCoords = aTexCoords;
    vec4 fragPos = modelMatrix * vec4(aPos, 1.0);
    FragPos = vec3(fragPos);

    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    TangentToWorldMatrix = mat3(T, B, N);

    gl_Position = projection * view * fragPos;
}
