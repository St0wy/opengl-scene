#version 310 es
precision highp float;

in vec2 TexCoords;
//in vec3 FragPos;

out vec4 FragColor;

uniform sampler2D texture_diffuse1;

void main()
{
	FragColor = texture(texture_diffuse1, TexCoords);
//	FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}