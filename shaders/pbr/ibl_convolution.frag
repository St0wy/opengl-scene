#version 430

out vec4 FragColor;

in vec3 WorldPos;

uniform samplerCube environmentMap;

const float PI = 3.14159265359;

void main()
{
	// The world vector acts as the normal of a tangent surface
	// from the origin, aligned to WorldPos. Given this normal, calculate all
	// incoming radiance of the environment. The result of this radiance
	// is the radiance of light coming from -Normal direction, which is what
	// we use in the PBR shader to sample irradiance.
	vec3 normal = normalize(WorldPos);

	vec3 irradiance = vec3(0.0);

	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, normal));
	up = normalize(cross(normal, right));

	float sampleDelta = 0.025;
	int nbrSamples = 0;
	for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			// Spherical to cartesian (in tangent space)
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			// Tangent space to world
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

			irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
			nbrSamples++;
		}
	}

	irradiance = PI * irradiance * (1.0 / float(nbrSamples));

	FragColor = vec4(irradiance, 1.0);
}
