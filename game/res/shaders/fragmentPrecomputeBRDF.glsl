#version 330 core

// ____ CONSTANTS ____
const float PI = 3.14159265359;
const uint SAMPLE_COUNT = 1024u;

// ____ GLOBAL VARIABLES ____
// Input data in world space from the vertex shader
in vec3 vsPosition;

// Output data
out vec4 glFragColor;


// ____ FUNCTION DEFINITIONS ____
float geometrySub(vec3 surfaceNormal, vec3 direction, float k)
{
	float normalDotDirection = max(dot(surfaceNormal, direction), 0.0);

	float numerator = normalDotDirection;
	float denominator = normalDotDirection * (1.0 - k) + k;
	return numerator / denominator;
}


/* Returns the relative surface area of the microfacets hidden behind other
 * microfacets from the viewer persepective */
float geometrySchlickGGX(vec3 surfaceNormal, vec3 viewDirection, vec3 lightDirection, float roughness)
{
	float k = 0.5 * roughness * roughness;
	return geometrySub(surfaceNormal, viewDirection, k) * geometrySub(surfaceNormal, lightDirection, k);
}


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


vec2 integrateBRDF(float nDotV, float roughness)
{
	vec3 v = vec3(sqrt(1.0 - nDotV * nDotV), 0.0, nDotV);

	float a = 0.0, b = 0.0;

	vec3 n = vec3(0.0, 0.0, 1.0);
	for(uint i = 0u; i < SAMPLE_COUNT; ++i)
	{
		vec2 xi = hammersleySquence(i, SAMPLE_COUNT);
		vec3 h = importanceSampleGGX(xi, n, roughness);
		vec3 l = normalize(2.0 * dot(v, h) * h - v);

		float nDotL = max(l.z, 0.0);
		float nDotH = max(h.z, 0.0);
		float vDotH = max(dot(v, h), 0.0);

		if (nDotL > 0.0) {
			float g = geometrySchlickGGX(n, v, l, roughness);
			float gVis = (g * vDotH) / (nDotH * nDotV);
			float fc = pow(1.0 - vDotH, 5.0);

			a += (1.0 - fc) * gVis;
			b += fc * gVis;
		}
	}
	a /= float(SAMPLE_COUNT);
	b /= float(SAMPLE_COUNT);

	return vec2(a, b);
}


// ____ MAIN PROGRAM ____
void main()
{
	float nDotV = gl_FragCoord.x / 512.0;
	float roughness = gl_FragCoord.y / 512.0;
	vec2 integratedBRDF = integrateBRDF(nDotV, roughness);

	glFragColor = vec4(integratedBRDF, 0.0, 1.0);
}
