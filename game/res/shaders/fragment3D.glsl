#version 330 core

// ____ CONSTANTS ____
const float PI					= 3.1415926535897932384626433832795;
const vec4	BASE_REFLECTIVITY	= vec4(0.04);
const vec2	INVERT_Y_TEXTURE	= vec2(1.0, -1.0);
const int	MAX_POINT_LIGHTS	= 4;
const float	SCREEN_GAMMA		= 2.2;	// Monitor is in sRGB color space


// ____ DATATYPES ____
struct PBRMetallicRoughness
{
	vec4 baseColorFactor;
	sampler2D baseColorTexture;
	float metallicFactor;
	float roughnessFactor;
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
	vec3	lightColor;
};

struct Attenuation
{
	float	constant;
	float	linear;
	float	exponential;
};

struct PointLight
{
	BaseLight	baseLight;
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
	float normalDotLightSqrd = dot(surfaceNormal, halfwayVector);
	normalDotLightSqrd *= normalDotLightSqrd;

	float numerator = roughness2;
	float denominator = normalDotLightSqrd * (roughness2 - 1) + 1;
	denominator = PI * denominator * denominator;

	return numerator / denominator;
}


float geometrySub(vec3 surfaceNormal, vec3 direction, float k)
{
	float normalDotDirection = max(dot(surfaceNormal, direction), 0.0);

	float numerator = normalDotDirection;
	float denominator = normalDotDirection * (1 - k) * k;
	return numerator / denominator;
}


/* Returns the relative surface area of the microfacets hidden behind other
 * microfacets from the viewer persepective */
float geometrySchlickGGX(vec3 surfaceNormal, vec3 viewDirection, vec3 lightDirection, float k)
{
	return geometrySub(surfaceNormal, viewDirection, k) * geometrySub(surfaceNormal, lightDirection, k);
}


/* Returns the ratio of light that gets reflected over the light that gets
 * refracted*/
vec4 fresnelSchlick(vec3 halfwayVector, vec3 viewDirection, vec4 reflectivity)
{
	return reflectivity + (vec4(1) - reflectivity) * pow(1 - dot(halfwayVector, viewDirection), 5);
}


/* Cook-Torrance BRDF function. It calculates how much a light ray contributes
 * to the reflected light of the given surface. */
vec4 calculateBRDF(vec3 viewDirection, vec3 lightDirection, vec3 surfaceNormal)
{
	float metallic		= uMaterial.pbrMetallicRoughness.metallicFactor;
	float roughness		= uMaterial.pbrMetallicRoughness.roughnessFactor;
	vec4 surfaceColor	= uMaterial.pbrMetallicRoughness.baseColorFactor;	// TODO: texture

	vec3 halfwayVector	= normalize(lightDirection + viewDirection);
	vec4 reflectivity	= mix(BASE_REFLECTIVITY, surfaceColor, metallic);
	float k				= (roughness + 1) * (roughness + 1) / 8;

	// Lambertian diffuse
	vec4 diffuseColor = surfaceColor / PI;

	// Cook-Torrance specular
	float n	= normalDistributionGGX(surfaceNormal, halfwayVector, roughness);
	vec4 f	= fresnelSchlick(halfwayVector, viewDirection, reflectivity);
	float g	= geometrySchlickGGX(surfaceNormal, viewDirection, lightDirection, k);

	vec4 numerator = n * f * g;
	float denominator = 4 * max(dot(viewDirection, surfaceNormal), 0.0) * max(dot(lightDirection, surfaceNormal), 0.0);
	vec4 specularColor = numerator / max(denominator, 0.001);

	// Combine both the diffuse color and the specular color
	vec4 diffuseRatio = (1.0 - f) * (1 - metallic);
	return diffuseRatio * diffuseColor + specularColor;
}


/* Calculates the radiance of the given point light */
vec4 calculateRadiance(PointLight pointLight, vec3 lightDirection, float lightDistance, vec3 surfaceNormal)
{
	// Calculate the attenuation of the point light
	float attenuation	= pointLight.attenuation.constant
						+ pointLight.attenuation.linear * lightDistance
						+ pointLight.attenuation.exponential * pow(lightDistance, 2);
	attenuation			= (attenuation != 0.0)? 1.0 / attenuation : 1.0;

	// Calculate the light radiance
	float cosTheta = max(dot(surfaceNormal, lightDirection), 0.0);
	return attenuation * vec4(pointLight.baseLight.lightColor, 1.0) * cosTheta;
}


/* Calculates the color of the current fragment point with all the PointLights
 * of the scene */
vec4 calculateDirectLight()
{
	// Calculate the view direction (the eye is in the center of the scene)
	vec3 viewDirection = normalize(-vsVertex.position);

	vec4 totalLightColor = vec4(0.0);
	for (int i = 0; i < vsNumPointLights; ++i) {
		// Calculate the light direction and distance from the current point
		vec3 lightDirection	= vsPointLightsPositions[i] - vsVertex.position;
		float lightDistance	= length(lightDirection);
		lightDirection		= normalize(lightDirection);

		// Calculate the light radiance
		vec4 radiance = calculateRadiance(uPointLights[i], lightDirection, lightDistance, vsVertex.normal);

		// Calculate the light reflections
		vec4 reflectedColor = calculateBRDF(viewDirection, lightDirection, vsVertex.normal);

		// Add the current ligth color
		totalLightColor += reflectedColor * radiance * max(dot(vsVertex.normal, lightDirection), 0.0);
	}

	return totalLightColor;
}


// ____ MAIN PROGRAM ____
void main()
{
	vec4 lightColor = calculateDirectLight();

	// Gamma correction
	lightColor = lightColor / (lightColor + vec4(1.0));
	glFragColor = pow(lightColor, vec4(1.0 / SCREEN_GAMMA));
}
