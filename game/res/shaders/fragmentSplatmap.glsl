#version 330 core

// ____ CONSTANTS ____
const float PI					= 3.1415926535897932384626433832795;
const vec3	BASE_REFLECTIVITY	= vec3(0.04);
const uint	MAX_LIGHTS			= 4u;
const uint	DIRECTIONAL_LIGHT	= 0u;
const uint	POINT_LIGHT			= 1u;
const uint	SPOT_LIGHT			= 2u;
const float	SCREEN_GAMMA		= 2.2;	// Monitor is in sRGB color space


// ____ DATATYPES ____
struct PBRMetallicRoughness
{
	vec4 baseColorFactor;
	bool useBaseColorTexture;
	sampler2D baseColorTexture;
	float metallicFactor;
	float roughnessFactor;
	bool useMetallicRoughnessTexture;
	sampler2D metallicRoughnessTexture;
};

struct Material
{
	PBRMetallicRoughness pbrMetallicRoughness;
	bool useNormalTexture;
	sampler2D normalTexture;
	float normalScale;
};

struct SplatmapMaterial
{
	int numMaterials;
	Material materials[4];
	sampler2D splatmapTexture;
};

struct BaseLight
{
	uint type;
	vec4 color;
	float intensity;
	float inverseRange;
	float lightAngleScale;
	float lightAngleOffset;
};


// ____ GLOBAL VARIABLES ____
// Input data in tangent space from the vertex shader
in FragmentIn
{
	vec3 position;
	vec2 texCoord0;
} fsVertex;

flat in uint fsNumLights;
in vec3 fsLightsPositions[MAX_LIGHTS];
in vec3 fsLightsDirections[MAX_LIGHTS];

// Uniform variables
layout (std140) uniform LightsBlock
{
	BaseLight uBaseLights[MAX_LIGHTS];
};

uniform SplatmapMaterial	uSMaterial;

// Output data
out vec4 glFragColor;


// ____ FUNCTION DEFINITIONS ____
/* Returns relative surface area of the microfacets aligned to the
 * halfwayVector */
float normalDistributionGGX(vec3 surfaceNormal, vec3 halfwayVector, float roughness)
{
	float roughness2 = roughness * roughness;
	float normalDotHalfway2 = max(dot(surfaceNormal, halfwayVector), 0.0);
	normalDotHalfway2 *= normalDotHalfway2;

	float numerator = roughness2;
	float denominator = normalDotHalfway2 * (roughness2 - 1.0) + 1.0;
	denominator = PI * denominator * denominator;

	return numerator / denominator;
}


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
	float k = roughness + 1.0;
	k = (k * k) / 8.0;

	return geometrySub(surfaceNormal, viewDirection, k) * geometrySub(surfaceNormal, lightDirection, k);
}


/* Returns the ratio of light that gets reflected over the light that gets
 * refracted*/
vec3 fresnelSchlick(float cosTheta, vec3 reflectivity)
{
	return reflectivity + (vec3(1.0) - reflectivity) * pow(1.0 - cosTheta, 5.0);
}


/* Calculates the radiance of the given light. If the light distance is
 * negative then distance attenuation will not be applied */
vec3 calculateRadiance(uint lightIndex, vec3 lightDirection, float lightDistance)
{
	// Calculate the attenuation of the light due to the distance
	float distanceAttenuation = clamp(1.0 - pow(lightDistance * uBaseLights[lightIndex].inverseRange, 4.0), 0.0, 1.0);
	distanceAttenuation /= lightDistance * lightDistance;

	bool hasDistanceAttenuation = lightDistance >= 0.0;
	distanceAttenuation = (distanceAttenuation * float(hasDistanceAttenuation)) + float(!hasDistanceAttenuation);

	// Calculate the angular attenuation of the light (for spot lights)
	float cd = dot(normalize(-fsLightsDirections[lightIndex]), lightDirection);
	float angularAttenuation = clamp(cd * uBaseLights[lightIndex].lightAngleScale + uBaseLights[lightIndex].lightAngleOffset, 0.0, 1.0);
	angularAttenuation *= angularAttenuation;

	bool hasAngularAttenuation = uBaseLights[lightIndex].type == SPOT_LIGHT;
	angularAttenuation = (angularAttenuation * float(hasAngularAttenuation)) + float(!hasAngularAttenuation);

	// Calculate the light radiance
	return distanceAttenuation * angularAttenuation * vec3(uBaseLights[lightIndex].color) * uBaseLights[lightIndex].intensity;
}


/* Calculates the color of the current fragment point with all the lights
 * of the scene */
vec3 calculateDirectLighting(vec3 albedo, float metallic, float roughness, vec3 surfaceNormal)
{
	vec3 totalLightColor = vec3(0.0);

	vec3 viewDirection		= normalize(-fsVertex.position);
	vec3 reflectivity		= mix(BASE_REFLECTIVITY, albedo, metallic);
	float normalDotView		= max(dot(surfaceNormal, viewDirection), 0.0);

	for (uint i = 0u; i < fsNumLights; ++i) {
		// Calculate the light direction and distance from the current point
		vec3 lightDirection1 = -fsLightsDirections[i];
		float lightDistance1 = -1.0;

		vec3 lightDirection2 = fsLightsPositions[i] - fsVertex.position;
		float lightDistance2 = length(lightDirection2);
		lightDirection2 /= lightDistance2;

		bool isDirectional = uBaseLights[i].type == DIRECTIONAL_LIGHT;
		vec3 lightDirection = (lightDirection1 * float(isDirectional)) + (lightDirection2 * float(!isDirectional));
		float lightDistance = (lightDistance1 * float(isDirectional)) + (lightDistance2 * float(!isDirectional));

		// Calculate the light radiance
		vec3 radiance = calculateRadiance(i, lightDirection, lightDistance);

		// Calculate the Lambertian diffuse color
		vec3 diffuseColor = albedo / PI;

		// Calculate the Cook-Torrance brdf specular color
		vec3 halfwayDirection = normalize(lightDirection + viewDirection);
		float halfwayDotView = clamp(dot(halfwayDirection, viewDirection), 0.0, 1.0);

		float n	= normalDistributionGGX(surfaceNormal, halfwayDirection, roughness);
		vec3 f	= fresnelSchlick(halfwayDotView, reflectivity);
		float g	= geometrySchlickGGX(surfaceNormal, viewDirection, lightDirection, roughness);

		float normalDotLight = max(dot(surfaceNormal, lightDirection), 0.0);
		vec3 specularColor = (n * f * g) / max(4 * normalDotView * normalDotLight, 0.001);

		// Add the current light color
		vec3 diffuseRatio = (1.0 - f) * (1.0 - metallic);
		totalLightColor += (diffuseRatio * diffuseColor + specularColor) * radiance * normalDotLight;
	}

	return totalLightColor;
}


/** Calculates how much the material located at the given index affects the
 * final material */
float getMaterialRate(vec4 splatmapColor, int materialIndex)
{
	switch (materialIndex) {
		case 0:		return splatmapColor.r;
		case 1:		return splatmapColor.g;
		case 2:		return splatmapColor.b;
		case 3:		return splatmapColor.a;
		default:	return 0.0;
	}
}


// ____ MAIN PROGRAM ____
void main()
{
	// Get the splatmap texture data for the current fragment
	vec4 splatmapColor = texture(uSMaterial.splatmapTexture, fsVertex.texCoord0);

	vec3 color = vec3(0.0);
	for (int i = 0; i < uSMaterial.numMaterials; ++i) {
		// Get the texture data of the material for the current fragment
		vec4 surfaceColor = (uSMaterial.materials[i].pbrMetallicRoughness.useBaseColorTexture)?
			pow(texture(uSMaterial.materials[i].pbrMetallicRoughness.baseColorTexture, fsVertex.texCoord0), vec4(2.2)) :
			vec4(1.0);
		surfaceColor *= uSMaterial.materials[i].pbrMetallicRoughness.baseColorFactor;

		vec4 metallicRoughnessColor = texture(uSMaterial.materials[i].pbrMetallicRoughness.metallicRoughnessTexture, fsVertex.texCoord0);

		float metallic = (uSMaterial.materials[i].pbrMetallicRoughness.useMetallicRoughnessTexture)?
			uSMaterial.materials[i].pbrMetallicRoughness.metallicFactor * metallicRoughnessColor.b :
			uSMaterial.materials[i].pbrMetallicRoughness.metallicFactor;

		float roughness = (uSMaterial.materials[i].pbrMetallicRoughness.useMetallicRoughnessTexture)?
			uSMaterial.materials[i].pbrMetallicRoughness.roughnessFactor * metallicRoughnessColor.g :
			uSMaterial.materials[i].pbrMetallicRoughness.roughnessFactor;

		vec3 surfaceNormal = (uSMaterial.materials[i].useNormalTexture)?
			normalize(2.0 * texture(uSMaterial.materials[i].normalTexture, fsVertex.texCoord0).rgb - 1.0)
				* vec3(uSMaterial.materials[i].normalScale, uSMaterial.materials[i].normalScale, 1.0) :
			vec3(0.0, 0.0, 1.0);

		// Calculate direct lighting
		vec3 directColor = calculateDirectLighting(surfaceColor.rgb, metallic, roughness, surfaceNormal);
		color += getMaterialRate(splatmapColor, i) * directColor;
	}

	// Gamma correction
	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0 / SCREEN_GAMMA));

	glFragColor = vec4(color, 1.0);
}
