#version 430

layout (location = 0) in vec3 aPos;
layout (location = 4) in mat4 modelMatrix;

uniform mat4 lightViewProjMatrix;

void main()
{
	gl_Position = lightViewProjMatrix * modelMatrix * vec4(aPos, 1.0);
}
