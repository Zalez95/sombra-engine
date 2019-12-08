#version 330 core

// ____ CONSTANTS ____
const float PI					= 3.1415926535897932384626433832795;
const vec3	BASE_REFLECTIVITY	= vec3(0.04);
const int	MAX_POINT_LIGHTS	= 4;
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
	bool useOcclusionTexture;
	sampler2D occlusionTexture;
	float occlusionStrength;
	bool useEmissiveTexture;
	sampler2D emissiveTexture;
	vec3 emissiveFactor;
	bool checkAlphaCutoff;
	float alphaCutoff;
};


struct PointLight
{
	vec3 color;
	float intensity;
	float inverseRange;
};


// ____ GLOBAL VARIABLES ____
// Input data in view tangent from the vertex shader
in FragmentIn
{
	vec3 position;
	vec2 texCoord0;
} fsVertex;

flat in int fsNumPointLights;
in vec3 fsPointLightsPositions[MAX_POINT_LIGHTS];

// Uniform variables
uniform Material	uMaterial;
uniform PointLight	uPointLights[MAX_POINT_LIGHTS];

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


/* Calculates the radiance of the given point light */
vec3 calculateRadiance(PointLight pointLight, float lightDistance)
{
	// Calculate the attenuation of the point light
	float attenuation = clamp(1.0 - pow(lightDistance * pointLight.inverseRange, 4.0), 0.0, 1.0);
	attenuation /= lightDistance * lightDistance;

	// Calculate the light radiance
	return attenuation * pointLight.color * pointLight.intensity;
}


/* Calculates the color of the current fragment point with all the PointLights
 * of the scene */
vec3 calculateDirectLighting(vec3 albedo, float metallic, float roughness, vec3 surfaceNormal)
{
	vec3 totalLightColor = vec3(0.0);

	vec3 viewDirection		= normalize(-fsVertex.position);
	vec3 reflectivity		= mix(BASE_REFLECTIVITY, albedo, metallic);
	float normalDotView		= max(dot(surfaceNormal, viewDirection), 0.0);

	for (int i = 0; i < fsNumPointLights; ++i) {
		// Calculate the light direction and distance from the current point
		vec3 lightDirection	= fsPointLightsPositions[i] - fsVertex.position;
		float lightDistance	= length(lightDirection);
		lightDirection /= lightDistance;

		// Calculate the light radiance
		vec3 radiance = calculateRadiance(uPointLights[i], lightDistance);

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


// ____ MAIN PROGRAM ____
void main()
{
	// Get the texture data for the current fragment
	vec4 surfaceColor = (uMaterial.pbrMetallicRoughness.useBaseColorTexture)?
		pow(texture(uMaterial.pbrMetallicRoughness.baseColorTexture, fsVertex.texCoord0), vec4(2.2)) :
		vec4(1.0);
	surfaceColor *= uMaterial.pbrMetallicRoughness.baseColorFactor;

	vec4 metallicRoughnessColor = texture(uMaterial.pbrMetallicRoughness.metallicRoughnessTexture, fsVertex.texCoord0);

	float metallic = (uMaterial.pbrMetallicRoughness.useMetallicRoughnessTexture)?
		uMaterial.pbrMetallicRoughness.metallicFactor * metallicRoughnessColor.b :
		uMaterial.pbrMetallicRoughness.metallicFactor;

	float roughness = (uMaterial.pbrMetallicRoughness.useMetallicRoughnessTexture)?
		uMaterial.pbrMetallicRoughness.roughnessFactor * metallicRoughnessColor.g :
		uMaterial.pbrMetallicRoughness.roughnessFactor;

	vec3 surfaceNormal = (uMaterial.useNormalTexture)?
		normalize(2.0 * texture(uMaterial.normalTexture, fsVertex.texCoord0).rgb - 1.0)
			* vec3(uMaterial.normalScale, uMaterial.normalScale, 1.0) :
		vec3(0.0, 0.0, 1.0);

	float surfaceAO = (uMaterial.useOcclusionTexture)?
		texture(uMaterial.occlusionTexture, fsVertex.texCoord0).r :
		0.0;

	vec3 emissiveColor = (uMaterial.useEmissiveTexture)?
		texture(uMaterial.emissiveTexture, fsVertex.texCoord0).rgb :
		vec3(1.0);
	emissiveColor *= uMaterial.emissiveFactor;

	// Check alpha cutoff
	if (uMaterial.checkAlphaCutoff && (surfaceColor.a < uMaterial.alphaCutoff)) {
		discard;
	}
	else {
		// Calculate direct lighting
		vec3 color = calculateDirectLighting(surfaceColor.rgb, metallic, roughness, surfaceNormal);

		// Set ambient occlusion
		color = (uMaterial.useOcclusionTexture)?
			mix(color, color * surfaceAO, uMaterial.occlusionStrength) :
			color;

		// Add emissive color
		color += emissiveColor;

		// Gamma correction
		color = color / (color + vec3(1.0));
		color = pow(color, vec3(1.0 / SCREEN_GAMMA));

		glFragColor = vec4(color, surfaceColor.a);
	}
}
