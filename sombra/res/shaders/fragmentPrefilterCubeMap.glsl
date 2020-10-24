#version 330 core

// ____ CONSTANTS ____
const float PI = 3.14159265359;
const uint SAMPLE_COUNT = 1024u;

// ____ GLOBAL VARIABLES ____
// Input data in world space from the vertex shader
in vec3 vsPosition;

// Uniform variables
uniform samplerCube uCubeMap;
uniform float uRoughness;

// Output data
out vec4 glFragColor;


// ____ FUNCTION DEFINITIONS ____
// See https://learnopengl.com/PBR/IBL/Specular-IBL
float vdcSequence(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}


/* Generates pseudo-random samples evenly distributed */
vec2 hammersleySquence(uint i, uint n)
{
	return vec2(float(i) / float(n), vdcSequence(i));
}


vec3 importanceSampleGGX(vec2 xi, vec3 normal, float roughness)
{
	float roughness2 = roughness * roughness;

	float phi = 2.0 * PI * xi.x;
	float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (roughness2 * roughness2 - 1.0) * xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	// from spherical coordinates to cartesian coordinates
	vec3 h;
	h.x = cos(phi) * sinTheta;
	h.y = sin(phi) * sinTheta;
	h.z = cosTheta;

	// from tangent-space vector to world-space sample vector
	vec3 up			= (abs(normal.z) < 0.999)? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent	= normalize(cross(up, normal));
	vec3 bitangent	= cross(normal, tangent);

	vec3 sampleVec = tangent * h.x + bitangent * h.y + normal * h.z;
	return normalize(sampleVec);
}


// ____ MAIN PROGRAM ____
void main()
{
	vec3 normal = normalize(vsPosition);
	vec3 r = normal;
	vec3 v = r;

	float totalWeight = 0.0;
	vec3 prefilteredColor = vec3(0.0);
	for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
		vec2 xi = hammersleySquence(i, SAMPLE_COUNT);
		vec3 h = importanceSampleGGX(xi, normal, uRoughness);
		vec3 l = normalize(2.0 * dot(v, h) * h - v);

		float nDotL = max(dot(normal, l), 0.0);
		if (nDotL > 0.0) {
			prefilteredColor	+= texture(uCubeMap, l).rgb * nDotL;
			totalWeight			+= nDotL;
		}
	}
	prefilteredColor = prefilteredColor / totalWeight;

	glFragColor = vec4(prefilteredColor, 1.0);
}
