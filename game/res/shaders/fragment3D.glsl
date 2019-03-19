#version 330 core

// ____ CONSTANTS ____
const float PI					= 3.1415926535897932384626433832795;
const vec3	BASE_REFLECTIVITY	= vec3(0.04);
const int	MAX_POINT_LIGHTS	= 4;
const float	SCREEN_GAMMA		= 2.2;	// Monitor is in sRGB color space


// ____ DATATYPES ____
struct PBRMetallicRoughness
{
	vec3 baseColorFactor;
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
	sampler2D normalTexture;
	sampler2D occlusionTexture;
	sampler2D emissiveTexture;
	vec3 emissiveFactor;
};

struct BaseLight
{
	vec3 lightColor;
};

struct Attenuation
{
	float constant;
	float linear;
	float exponential;
};

struct PointLight
{
	BaseLight baseLight;
	Attenuation attenuation;
};


// ____ GLOBAL VARIABLES ____
// Input data in view space from the vertex shader
in VertexData
{
	vec3 position;
	vec3 normal;
	vec2 texCoord0;
} vsVertex;

flat in int vsNumPointLights;
in vec3 vsPointLightsPositions[MAX_POINT_LIGHTS];

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
	float normalDotLight2 = max(dot(surfaceNormal, halfwayVector), 0.0);
	normalDotLight2 *= normalDotLight2;

	float numerator = roughness2;
	float denominator = normalDotLight2 * (roughness2 - 1.0) + 1.0;
	denominator = PI * denominator * denominator;

	return numerator / denominator;
}


float geometrySub(vec3 surfaceNormal, vec3 direction, float k)
{
	float normalDotDirection = max(dot(surfaceNormal, direction), 0.0);

	float numerator = normalDotDirection;
	float denominator = normalDotDirection * (1.0 - k) * k;
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
vec3 fresnelSchlick(vec3 halfwayVector, vec3 viewDirection, vec3 reflectivity)
{
	float cosTheta = max(dot(halfwayVector, viewDirection), 0.0);
	return reflectivity + (vec3(1.0) - reflectivity) * pow(1.0 - cosTheta, 5.0);
}


/* Calculates the radiance of the given point light */
vec3 calculateRadiance(PointLight pointLight, float lightDistance)
{
	// Calculate the attenuation of the point light
	float attenuation	= pointLight.attenuation.constant
						+ pointLight.attenuation.linear * lightDistance
						+ pointLight.attenuation.exponential * lightDistance * lightDistance;
	attenuation			= (attenuation != 0.0)? 1.0 / attenuation : 1.0;

	// Calculate the light radiance
	return attenuation * pointLight.baseLight.lightColor;
}


/* Calculates the color of the current fragment point with all the PointLights
 * of the scene */
vec3 calculateDirectLight()
{
	vec3 totalLightColor = vec3(0.0);

	vec3 surfaceNormal = vsVertex.normal;

	float metallic	= uMaterial.pbrMetallicRoughness.metallicFactor;
	float roughness	= uMaterial.pbrMetallicRoughness.roughnessFactor;
	if (uMaterial.pbrMetallicRoughness.useMetallicRoughnessTexture) {
		vec4 metallicRoughnessColor = texture(uMaterial.pbrMetallicRoughness.baseColorTexture, vsVertex.texCoord0);
		metallic	*= metallicRoughnessColor.b;
		roughness	*= metallicRoughnessColor.g;
	}

	vec3 surfaceColor = uMaterial.pbrMetallicRoughness.baseColorFactor;
	if (uMaterial.pbrMetallicRoughness.useBaseColorTexture) {
		surfaceColor *= texture(uMaterial.pbrMetallicRoughness.baseColorTexture, vsVertex.texCoord0).rgb;
	}

	float surfaceAO = texture(uMaterial.occlusionTexture, vsVertex.texCoord0).r;

	vec3 viewDirection		= normalize(-vsVertex.position);
	vec3 reflectivity		= mix(BASE_REFLECTIVITY, surfaceColor, metallic);
	float normalDotView		= max(dot(surfaceNormal, viewDirection), 0.0);

	for (int i = 0; i < vsNumPointLights; ++i) {
		// Calculate the light direction and distance from the current point
		vec3 lightDirection	= vsPointLightsPositions[i] - vsVertex.position;
		float lightDistance	= length(lightDirection);
		lightDirection		= normalize(lightDirection);

		// Calculate the light radiance
		vec3 radiance = calculateRadiance(uPointLights[i], lightDistance);

		// Calculate the Lambertian diffuse color
		vec3 diffuseColor = surfaceColor / PI;

		// Calculate the Cook-Torrance brdf specular color
		vec3 halfwayVector = normalize(lightDirection + viewDirection);

		float n	= normalDistributionGGX(surfaceNormal, halfwayVector, roughness);
		vec3 f	= fresnelSchlick(halfwayVector, viewDirection, reflectivity);
		float g	= geometrySchlickGGX(surfaceNormal, viewDirection, lightDirection, roughness);

		float normalDotLight = max(dot(surfaceNormal, lightDirection), 0.0);
		vec3 specularColor = (n * f * g) / max(4 * normalDotView * normalDotLight, 0.001);

		// Add the current light color
		vec3 diffuseRatio = (1.0 - f) * (1.0 - metallic);
		totalLightColor += (diffuseRatio * diffuseColor + specularColor) * radiance * normalDotLight;
	}

	// Add ambient color
	totalLightColor += vec3(0.03) * surfaceAO * surfaceColor;

	return totalLightColor;
}


// ____ MAIN PROGRAM ____
void main()
{
	vec3 lightColor = calculateDirectLight();

	// Gamma correction
	lightColor = lightColor / (lightColor + vec3(1.0));
	lightColor = pow(lightColor, vec3(1.0 / SCREEN_GAMMA));

	glFragColor = vec4(lightColor, 1.0);
}
