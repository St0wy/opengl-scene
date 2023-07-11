#version 430

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in mat4 modelMatrix;

uniform mat4 lightViewProjMatrix;

void main()
{
	gl_Position = lightViewProjMatrix * modelMatrix * vec4(aPos, 1.0);
}
