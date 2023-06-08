#version 310 es
precision highp float;

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform samplerCube skybox;
uniform mat4 view;

void main()
{
	float ratio = 1.0 / 1.52;
	vec3 viewDir = normalize(FragPos);
	vec3 reflexion = refract(viewDir, normalize(Normal), ratio);
	vec3 worldReflexion = inverse(mat3(view)) * reflexion;
	FragColor = vec4(texture(skybox, worldReflexion).rgb, 1.0);
}