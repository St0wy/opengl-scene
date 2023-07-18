#version 430

in vec2 TexCoords;

layout (location = 0) out vec4 FragColor;

uniform sampler2D gPositionAmbientOcclusion;
uniform sampler2D gImage;

uniform float fogDensity;
uniform vec3 fogColor;

void main()
{
	vec3 fragPos = texture(gPositionAmbientOcclusion, TexCoords).rgb;
	vec3 imageColor = texture(gImage, TexCoords).rgb;
	float depth = fragPos.z;

	float depthDensity = depth * fogDensity;
	float squaredDepthDensity = depthDensity * depthDensity;
	float fogFactor = pow(2.0, -squaredDepthDensity);

	vec3 color = mix(imageColor, fogColor, fogFactor);

	//	color = fragPos;
	//	color = vec3(fogDensity);
//	color = imageColor;
	FragColor = vec4(color, 1.0);
}
