#version 430

const int SAMPLES_COUNT = 64;
const float RANDOM_TEXTURE_SIZE = 4.0;
const float RADIUS = 0.5;
const float BIAS = 0.025;

layout (location = 0) out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPositionAmbientOcclusion;
uniform sampler2D gNormalRoughness;
uniform sampler2D texNoise;

uniform vec3 samples[SAMPLES_COUNT];
uniform vec2 screenSize;

layout (std140, binding = 0) uniform Matrices
{
	mat4 projection;
	mat4 view;
};

void main()
{
	vec3 fragPos = texture(gPositionAmbientOcclusion, TexCoords).rgb;
	vec3 normal = texture(gNormalRoughness, TexCoords).rgb;

	vec2 noiseScale = screenSize / RANDOM_TEXTURE_SIZE;
	vec3 randomVec = texture(texNoise, TexCoords * noiseScale).xyz;

	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for (int i = 0; i < SAMPLES_COUNT; i++)
	{
		// From tangent space to view space
		vec3 samplePos = TBN * samples[i];
		samplePos = fragPos.xyz + samplePos * RADIUS;

		vec4 offset = vec4(samplePos, 1.0);
		// from view to clip-space
		offset = projection * offset;
		// perspective divide
		offset.xyz /= offset.w;
		// transform to range 0.0 - 1.0
		offset.xyz = offset.xyz * 0.5 + 0.5;

		float sampleDepth = texture(gPositionAmbientOcclusion, TexCoords).z;
		float rangeCheck = smoothstep(0.0, 1.0, RADIUS / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= samplePos.z + BIAS ? 1.0 : 0.0) * rangeCheck;
	}
	occlusion = 1.0 - (occlusion / SAMPLES_COUNT);

	FragColor = occlusion;
}
