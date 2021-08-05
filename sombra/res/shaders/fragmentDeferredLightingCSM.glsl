#version 330 core

// ____ CONSTANTS ____
const float PI					= 3.1415926535897932384626433832795;
const vec3	BASE_REFLECTIVITY	= vec3(0.04);
const float MAX_REFLECTION_LOD	= 4.0;	// Maximum LOD of the prefilter map
const uint	MAX_LIGHTS			= 32u;
const uint	DIRECTIONAL_LIGHT	= 0u;
const uint	POINT_LIGHT			= 1u;
const uint	SPOT_LIGHT			= 2u;
const float	MAX_SHADOW_BIAS		= 0.005;
const float	MIN_SHADOW_BIAS		= 0.001;
const uint	MAX_CASCADES		= 6u;
const vec2 POISSON_DISK[16] = vec2[](
	vec2(0.282571, 0.023957), vec2(0.792657, 0.945738),
	vec2(0.922361, 0.411756), vec2(0.165838, 0.552995),
	vec2(0.566027, 0.216651), vec2(0.335398, 0.783654),
	vec2(0.0190741, 0.318522), vec2(0.647572, 0.581896),
	vec2(0.916288, 0.0120243), vec2(0.0278329, 0.866634),
	vec2(0.398053, 0.4214), vec2(0.00289926, 0.051149),
	vec2(0.517624, 0.989044), vec2(0.963744, 0.719901),
	vec2(0.76867, 0.018128), vec2(0.684194, 0.167302)
);


// ____ DATATYPES ____
struct BaseLight
{
	uint type;
	vec3 color;
	float intensity;
	float range;
	float lightAngleScale;
	float lightAngleOffset;
};


// ____ GLOBAL VARIABLES ____
// Input data from the vertex shader
in vec3 vsPosition;

// Uniform variables
uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform vec3 uViewPosition;
uniform BaseLight uBaseLight;
uniform uint uNumCascades;
uniform mat4 uShadowVPMatrices[MAX_CASCADES];
uniform float uCascadesZFar[MAX_CASCADES];

uniform sampler2D uPosition;
uniform sampler2D uNormal;
uniform sampler2D uAlbedo;
uniform sampler2D uMaterial;
uniform sampler2DArray uShadow;

// Output data
layout (location = 0) out vec4 oColor;
layout (location = 1) out vec4 oBright;


// ____ FUNCTION DEFINITIONS ____
/** Random function in the range [0,1] */
float rand(vec2 seed)
{
	return fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453);
}


/* Returns relative surface area of the microfacets aligned to the
 * halfwayVector */
float normalDistributionGGX(vec3 normalDirection, vec3 halfwayVector, float roughness)
{
	float roughness2 = roughness * roughness;
	float normalDotHalfway2 = max(dot(normalDirection, halfwayVector), 0.0);
	normalDotHalfway2 *= normalDotHalfway2;

	float numerator = roughness2;
	float denominator = normalDotHalfway2 * (roughness2 - 1.0) + 1.0;
	denominator = PI * denominator * denominator;

	return numerator / denominator;
}


float geometrySub(vec3 normalDirection, vec3 direction, float k)
{
	float normalDotDirection = max(dot(normalDirection, direction), 0.0);

	float numerator = normalDotDirection;
	float denominator = normalDotDirection * (1.0 - k) + k;
	return numerator / denominator;
}


/* Returns the relative surface area of the microfacets hidden behind other
 * microfacets from the viewer persepective */
float geometrySchlickGGX(vec3 normalDirection, vec3 viewDirection, vec3 lightDirection, float roughness)
{
	float k = roughness + 1.0;
	k = (k * k) / 8.0;

	return geometrySub(normalDirection, viewDirection, k) * geometrySub(normalDirection, lightDirection, k);
}


/* Returns the ratio of light that gets reflected over the light that gets
 * refracted*/
vec3 fresnelSchlick(float cosTheta, vec3 reflectivity)
{
	return reflectivity + (vec3(1.0) - reflectivity) * pow(1.0 - cosTheta, 5.0);
}


/* Calculates the radiance of the given light. If the light distance is
 * negative then distance attenuation will not be applied */
vec4 calculateRadiance(vec3 lightDirection, float lightDistance, vec3 lightForward)
{
	// Calculate the attenuation of the light due to the distance
	float distanceAttenuation = clamp(1.0 - pow(lightDistance / uBaseLight.range, 4.0), 0.0, 1.0);
	distanceAttenuation /= lightDistance * lightDistance;

	bool hasDistanceAttenuation = lightDistance >= 0.0;
	distanceAttenuation = (distanceAttenuation * float(hasDistanceAttenuation)) + float(!hasDistanceAttenuation);

	// Calculate the angular attenuation of the light (for spot lights)
	float cd = dot(-lightForward, lightDirection);
	float angularAttenuation = clamp(cd * uBaseLight.lightAngleScale + uBaseLight.lightAngleOffset, 0.0, 1.0);
	angularAttenuation *= angularAttenuation;

	bool hasAngularAttenuation = uBaseLight.type == SPOT_LIGHT;
	angularAttenuation = (angularAttenuation * float(hasAngularAttenuation)) + float(!hasAngularAttenuation);

	// Calculate the light radiance
	return distanceAttenuation * angularAttenuation * vec4(uBaseLight.color, 1.0) * uBaseLight.intensity;
}


/** Checks wether the current fragment is in shadow or not */
float calculateShadow(vec3 position, vec3 normal, vec3 lightDirection)
{
	// Calculate the shadow bias
	float shadowBias = max(MAX_SHADOW_BIAS * (1.0 - dot(normal, lightDirection)), MIN_SHADOW_BIAS);

	// Calculate the cascade to use
	uint cascadeIndex = uNumCascades - 1u;
	float depth = -(uViewMatrix * vec4(position, 1.0)).z;	// View pointing towards -Z
	for (uint i = 0u; i < uNumCascades; ++i) {
		if (depth <= uCascadesZFar[i]) {
			cascadeIndex = i;
			break;
		}
	}

	// Calculate the current shadow depth
	vec4 shadowPosition = uShadowVPMatrices[cascadeIndex] * vec4(position, 1.0);
	vec3 shadowTexCoord = shadowPosition.xyz / shadowPosition.w;	// Perspective divide
	shadowTexCoord = shadowTexCoord * 0.5 + 0.5;					// [0,1] range
	float currentDepth = shadowTexCoord.z;

	// Compare the current depth with the shadow map using PCF with Poisson Sampling
	float shadow = 0.0;
	for (uint i = 0u; i < 4u; ++i) {
		uint randomIndex = uint(16.0 * rand(i * gl_FragCoord.xy)) % 16u;	// This always returns the same value for a screen coordinate
		vec3 shadowTexCoord = vec3(shadowTexCoord.xy + POISSON_DISK[randomIndex] / 700.0, float(cascadeIndex));
		float closestDepth = texture(uShadow, shadowTexCoord).r;

		bool inShadow = (currentDepth <= 1.0) && (currentDepth - shadowBias > closestDepth);
		shadow += float(inShadow) * 0.25;
	}

	return shadow;
}


/* Calculates the color of the current fragment point with the current light
 * of the scene */
vec4 calculateLighting(vec3 position, vec3 normal, vec3 albedo, float metallic, float roughness)
{
	vec4 lightColor = vec4(0.0);

	vec3 normalDirection	= normalize(normal);
	vec3 viewDirection		= normalize(uViewPosition - position);
	vec3 reflectivity		= mix(BASE_REFLECTIVITY, albedo, metallic);
	float normalDotView		= max(dot(normalDirection, viewDirection), 0.0);
	vec3 lightPosition		= vec3(uModelMatrix[3]);
	vec3 lightForward		= normalize(mat3(uModelMatrix) * vec3(0.0, 0.0,-1.0));	// Point towards -Z so it matches the camera direction

	// Calculate the light direction and distance from the current point
	vec3 lightDirection = -lightForward;
	float lightDistance = 1.0;
	if (lightDistance < uBaseLight.range) {
		// Calculate the light radiance
		vec4 radiance = calculateRadiance(lightDirection, lightDistance, lightForward);

		// Calculate the Lambertian diffuse color
		vec3 diffuseColor = albedo / PI;

		// Calculate the Cook-Torrance brdf specular color
		vec3 halfwayDirection = normalize(lightDirection + viewDirection);
		float halfwayDotView = clamp(dot(halfwayDirection, viewDirection), 0.0, 1.0);

		float n	= normalDistributionGGX(normalDirection, halfwayDirection, roughness);
		vec3 f	= fresnelSchlick(halfwayDotView, reflectivity);
		float g	= geometrySchlickGGX(normalDirection, viewDirection, lightDirection, roughness);

		float normalDotLight = max(dot(normalDirection, lightDirection), 0.0);
		vec3 specularColor = (n * f * g) / max(4 * normalDotView * normalDotLight, 0.001);

		// Add the current light color if it's being lit
		vec3 diffuseRatio = (vec3(1.0) - f) * (1.0 - metallic);
		lightColor = vec4(diffuseRatio * diffuseColor + specularColor, 1.0) * radiance * normalDotLight;

		// Calculate shadow mapping
		lightColor *= (1.0 - calculateShadow(position, normal, lightDirection));
	}

	return lightColor;
}


// ____ MAIN PROGRAM ____
void main()
{
	// Extract the data from the GBuffer
	vec2 resolution = textureSize(uPosition, 0);
	vec2 texCoords = gl_FragCoord.xy / resolution;

	vec3 position = texture(uPosition, texCoords).rgb;
	vec3 normal = texture(uNormal, texCoords).rgb;
	vec3 albedo = texture(uAlbedo, texCoords).rgb;

	vec4 material = texture(uMaterial, texCoords);
	float metallic = material.r;
	float roughness = material.g;

	// Calculate the output color
	oColor = calculateLighting(position, normal, albedo, metallic, roughness);

	// Calculate the output bright color
	bool isBright = dot(oColor.rgb, vec3(0.2126, 0.7152, 0.0722)) > 1.0;
	oBright = oColor * float(isBright) + vec4(0.0, 0.0, 0.0, 1.0) * float(!isBright);
}
