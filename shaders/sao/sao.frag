#version 430

const float FAR_PLANE_Z = -300.0;

// Total number of direct samples to take at each pixel
const int NUM_SAMPLES = 11;

// This is the number of turns around the circle that the spiral pattern makes.
// This should be prime to prevent taps from lining up.  This particular choice
// was tuned for NUM_SAMPLES == 9
const int NUM_SPIRAL_TURNS = 7;

// If using depth mip levels, the log of the maximum pixel offset before we need
// to switch to a lower miplevel to maintain reasonable spatial locality in the
// cache If this number is too small (< 3), too many taps will land in the same
// pixel, and we'll get bad variance that manifests as flashing. If it is too
// high (> 5), we'll get bad performance because we're not using the MIP levels
// effectively
const int LOG_MAX_OFFSET = 3;

// This must be less than or equal to the MAX_MIP_LEVEL defined in SSAO.cpp
const int MAX_MIP_LEVEL = 5;

struct TapLocationResult
{
	vec2 unitOffset;
	float screenSpaceRotation;
};

layout (location = 0) out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPositionAmbientOcclusion;
uniform sampler2D gNormalRoughness;

// World-space AO radius in scene units (r).  e.g., 1.0m
uniform float radius;
float radius2 = radius * radius;

// The height in pixels of a 1m object if viewed from 1m away.
// You can compute it from your projection matrix. The actual value is
// just a scale factor on radius; you can simply hardcode this to a constant
// (~500) and make your radius value unitless (...but resolution dependent.)
// -height / (2.0 * tan(verticalFieldOfView * 0.5))
uniform float projScale;

// Bias to avoid AO in smooth corners, e.g., 0.01m
uniform float bias;

// intensity / radius ^ 6
uniform float intensityDivR6;

float ViewSpaceZToKey(float z);
vec2 PackKey(float key);
float SampleAO(ivec2 intTexCoords, vec3 fragPos, vec3 normal, float screenSpaceDiskRadius, 
	int tapIndex, float randomPatternRotationAngle);
TapLocationResult TapLocation(int sampleNumber, float spinAngle, out float screenSpaceRotation);
vec3 GetOffsetPosition(ivec2 intTexCoords, vec2 unitOffset, float screenSpaceRotation);

void main()
{
	// View-space fragment positions and normals
	vec3 fragPos = texture(gPositionAmbientOcclusion, TexCoords).rgb;
	vec3 normal = texture(gNormalRoughness, TexCoords).rgb;

	ivec2 intTexCoords = ivec2(gl_FragCoord.xy);

//	FragColor.gb = PackKey(ViewSpaceZToKey(fragPos.z));

	// Hash function used in the HPG12 AlchemyAO paper
	float randomPatternRotationAngle = (3 * intTexCoords.x ^ intTexCoords.y + intTexCoords.x * intTexCoords.y) * 10;
	FragColor = randomPatternRotationAngle;
	return;

	float screenSpaceDiskRadius = -projScale * radius / fragPos.z;

	float sum = 0.0;
	for (int i = 0; i < NUM_SAMPLES; ++i) 
	{
		sum += SampleAO(intTexCoords, fragPos, normal, screenSpaceDiskRadius, i, randomPatternRotationAngle);
	}

	float ambientOcclusion = max(0.0, 1.0 - sum * intensityDivR6 * (5.0 / NUM_SAMPLES));

	if (abs(dFdx(fragPos.z)) < 0.02) {
		ambientOcclusion -= dFdx(ambientOcclusion) * ((intTexCoords.x & 1) - 0.5);
	}
	if (abs(dFdy(fragPos.z)) < 0.02) {
		ambientOcclusion -= dFdy(ambientOcclusion) * ((intTexCoords.y & 1) - 0.5);
	}

	FragColor.r = ambientOcclusion;
}

// Used for packing Z into the GB channels
float ViewSpaceZToKey(float z)
{ 
	return clamp(z * (1.0 / FAR_PLANE_Z), 0.0, 1.0); 
}

vec2 PackKey(float key) 
{
	// Round to the nearest 1/256.0
	float temp = floor(key * 256.0);

	vec2 p;
	// Integer part
	p.x = temp * (1.0 / 256.0);

	// Fractional part
	p.y = key * 256.0 - temp;

	return p;
}

// Compute the occlusion due to sample with index \a i about the pixel at \a
// ssC that corresponds to camera-space point \a C with unit normal \a n_C,
// using maximum screen-space sampling radius \a ssDiskRadius
// Note that units of H() in the HPG12 paper are meters, not
// unitless. The whole falloff/sampling function is therefore
// unitless. In this implementation, we factor out (9 / radius).
// Four versions of the falloff function are implemented below
float SampleAO(ivec2 intTexCoords, vec3 fragPos, vec3 normal, float screenSpaceDiskRadius, 
	int tapIndex, float randomPatternRotationAngle) 
{
	// Offset on the unit disk, spun for this pixel
	float screenSpaceRotation;
	TapLocationResult tapLocationResult = TapLocation(tapIndex, randomPatternRotationAngle, screenSpaceRotation);
	screenSpaceRotation *= screenSpaceDiskRadius;

	vec3 cameraSpaceOccludingPoint = GetOffsetPosition(intTexCoords, tapLocationResult.unitOffset, screenSpaceRotation);

	vec3 fragToOccludingPoint = cameraSpaceOccludingPoint - fragPos;

	float vv = dot(fragToOccludingPoint, fragToOccludingPoint);
	float vn = dot(fragToOccludingPoint, normal);

	const float epsilon = 0.01;

	// A: From the HPG12 paper
	// Note large epsilon to avoid overdarkening within cracks
	// return float(vv < radius2) * max((vn - bias) / (epsilon + vv), 0.0) *
	// radius2 * 0.6;

	// B: Smoother transition to zero (lowers contrast, smoothing out corners).
	// [Recommended]
	float f = max(radius2 - vv, 0.0);
	return f * f * f * max((vn - bias) / (epsilon + vv), 0.0);

	// C: Medium contrast (which looks better at high radii), no division.  Note
	// that the contribution still falls off with radius^2, but we've adjusted the
	// rate in a way that is more computationally efficient and happens to be
	// aesthetically pleasing. return 4.0 * max(1.0 - vv * invRadius2, 0.0) *
	// max(vn - bias, 0.0);

	// D: Low contrast, no division operation
	// return 2.0 * float(vv < radius * radius) * max(vn - bias, 0.0);
}

// Returns a unit vector and a screen-space radius for the tap on a unit disk
// (the caller should scale by the actual disk radius)
TapLocationResult TapLocation(int sampleNumber, float spinAngle, out float screenSpaceRotation)
{
	// Radius relative to ssR
	float alpha = (float(sampleNumber) + 0.5) * (1.0 / NUM_SAMPLES);
	float angle = alpha * (NUM_SPIRAL_TURNS * 6.28) + spinAngle;
	
	TapLocationResult result;
	result.unitOffset = vec2(cos(angle), sin(angle));
	result.screenSpaceRotation = alpha;
	return result;
}

// Read the camera-space position of the point at screen-space pixel ssP + unitOffset * ssR.  Assumes length(unitOffset) == 1
vec3 GetOffsetPosition(ivec2 intTexCoords, vec2 unitOffset, float screenSpaceRotation) 
{
	ivec2 offsetedTexCoords = ivec2(screenSpaceRotation * unitOffset) + intTexCoords;

	return texture(gPositionAmbientOcclusion, offsetedTexCoords).rgb;
}
