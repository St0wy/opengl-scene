#version 430

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrBuffer;

void main()
{
	const float gamma = 2.2;
	vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;

	// reinhard tone mapping
	//	vec3 mapped = hdrColor / (hdrColor + vec3(1.0));

	// Narkowicz ACES tone mapping
	vec3 mapped = (hdrColor * (2.51f * hdrColor + 0.03f)) / (hdrColor * (2.43f * hdrColor + 0.59f) + 0.14f);
	mapped = clamp(mapped, vec3(0.0), vec3(1.0));

	// gamma correction
	mapped = pow(mapped, vec3(1.0 / gamma));

	FragColor = vec4(mapped, 1.0);
}