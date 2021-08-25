#version 330 core

// ____ CONSTANTS ____
const uint	NUM_SAMPLES	= 64u;
const float	RADIUS		= 1.5;
const float	BIAS		= 0.005;


// ____ GLOBAL VARIABLES ____
// Input data from the vertex shader
in vec3 vsPosition;

// Uniform variables
uniform sampler2D uPosition;
uniform sampler2D uNormal;
uniform sampler2D uRotationNoise;
uniform mat4 uFragmentViewMatrix;
uniform mat4 uFragmentProjectionMatrix;
uniform vec3 uHemisphereSamples[NUM_SAMPLES];

// Output data
out float glFragColor;


// ____ FUNCTION DEFINITIONS ____
/** Calculates the TBN matrix from the given normal vector and a random one */
mat3 calculateTBNMatrix(vec3 randomVector, vec3 normal)
{
	// Calculate the tangent and normal vectors in world space
	vec3 T = randomVector;
	vec3 N = normal;

	// Fix normalization issues so T and N are orthogonal
	T = normalize(T - dot(T, N) * N);

	// Calculate the bit-tangent vector in world space
	vec3 B = cross(N, T);

	// Calculate the TBN matrix
	return mat3(T, B, N);
}


// ____ MAIN PROGRAM ____
void main()
{
	// Extract the data from the GBuffer
	vec2 texCoords = (0.5 * vsPosition + 0.5).xy;
	vec2 noiseScale = textureSize(uPosition, 0) / 4.0;

	vec3 position = texture(uPosition, texCoords).rgb;
	vec3 normal = texture(uNormal, texCoords).rgb;
	vec3 noise = texture(uRotationNoise, texCoords * noiseScale).rgb;

	// Calculate the position and normal in view space
	position = vec3(uFragmentViewMatrix * vec4(position, 1.0));
	normal = vec3(uFragmentViewMatrix * vec4(normal, 0.0));
	normal = normalize(normal);

	// Calculate the TBN matrix
	mat3 tbnMatrix = calculateTBNMatrix(noise, normal);

	// Calculate the occlusion
	float occlusion = 0.0;
	for (uint i = 0u; i < NUM_SAMPLES; ++i) {
		// Calculate a random position based on the current one and the sample uniform
		vec3 samplePosition = tbnMatrix * uHemisphereSamples[i];	// Transform the sample to world-space
		samplePosition = position + RADIUS * samplePosition;		// Add the previous value to the current position scaled by the radius

		// Get the sample position in the uPosition texture
		vec4 projection = uFragmentProjectionMatrix * vec4(samplePosition, 1.0);
		projection.xyz = projection.xyz / projection.w;
		projection.xyz = projection.xyz * 0.5 + 0.5;

		vec2 sampleTexCoords = projection.xy;
		vec3 sampleTexturePosition = texture(uPosition, sampleTexCoords).rgb;
		sampleTexturePosition = vec3(uFragmentViewMatrix * vec4(sampleTexturePosition, 1.0));

		// Compare its values
		float rangeCheck = smoothstep(0.0, 1.0, RADIUS / abs(position.z - sampleTexturePosition.z));
		occlusion += rangeCheck * float(sampleTexturePosition.z >= samplePosition.z + BIAS);
	}

	// Normalize the occlusion
	occlusion = 1.0 - (occlusion / NUM_SAMPLES);

	glFragColor = pow(occlusion, 2.0);
}
