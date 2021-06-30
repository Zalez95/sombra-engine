#version 330 core

// ____ CONSTANTS ____
const float PI					= 3.1415926535897932384626433832795;
const vec3	BASE_REFLECTIVITY	= vec3(0.04);
const float MAX_REFLECTION_LOD	= 4.0;	// Maximum LOD of the prefilter map
const uint	MAX_LIGHTS			= 32u;
const uint	DIRECTIONAL_LIGHT	= 0u;
const uint	POINT_LIGHT			= 1u;
const uint	SPOT_LIGHT			= 2u;
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
	vec3 position;
	uint type;
	vec3 direction;
	int shadowIndices;
	vec4 color;
	float intensity;
	float range;
	float lightAngleScale;
	float lightAngleOffset;
};


// ____ GLOBAL VARIABLES ____
// Input data from the vertex shader
in vec3 vsPosition;

// Uniform variables
uniform uint uNumLights;
layout (std140) uniform LightsBlock
{
	BaseLight uBaseLights[MAX_LIGHTS];
};

uniform vec3 uViewPosition;						// Camera position in World space

uniform sampler2D uShadows;
uniform samplerCube uIrradianceMap;
uniform samplerCube uPrefilterMap;
uniform sampler2D uBRDFMap;
uniform sampler2D uPosition;
uniform sampler2D uNormal;
uniform sampler2D uAlbedo;
uniform sampler2D uMaterial;
uniform sampler2D uEmissive;

// Output data
layout (location = 0) out vec4 oColor;
layout (location = 1) out vec4 oBright;


// ____ FUNCTION DEFINITIONS ____
/** Random function in the range [0,1] */
float rand(vec2 seed)
{
	return fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453);
}


/* Calculates wether the given point is in shadow or not */
float calculateShadow(uint shadowIndex, vec4 shadows)
{
	uint q = shadowIndex / 4u;
	uint r = shadowIndex % 4u;

	uint value = (q == 0u)? uint(shadows.r) : (q == 1u)? uint(shadows.g) : (q == 2u)? uint(shadows.b) : uint(shadows.a);
	uint v1 = value / 16777216u;
	uint v2 = (value - v1) / 65536u;
	uint v3 = (value - v1 - v2) / 256u;
	uint v4 = value - v1 - v2 - v3;
	value = (r == 0u)? v1 : (r == 1u)? v2 : (r == 2u)? v3 : v4;

	return float(value) / 255.0;
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
vec3 calculateRadiance(uint lightIndex, vec3 lightDirection, float lightDistance)
{
	// Calculate the attenuation of the light due to the distance
	float distanceAttenuation = clamp(1.0 - pow(lightDistance / uBaseLights[lightIndex].range, 4.0), 0.0, 1.0);
	distanceAttenuation /= lightDistance * lightDistance;

	bool hasDistanceAttenuation = lightDistance >= 0.0;
	distanceAttenuation = (distanceAttenuation * float(hasDistanceAttenuation)) + float(!hasDistanceAttenuation);

	// Calculate the angular attenuation of the light (for spot lights)
	float cd = dot(-uBaseLights[lightIndex].direction, lightDirection);
	float angularAttenuation = clamp(cd * uBaseLights[lightIndex].lightAngleScale + uBaseLights[lightIndex].lightAngleOffset, 0.0, 1.0);
	angularAttenuation *= angularAttenuation;

	bool hasAngularAttenuation = uBaseLights[lightIndex].type == SPOT_LIGHT;
	angularAttenuation = (angularAttenuation * float(hasAngularAttenuation)) + float(!hasAngularAttenuation);

	// Calculate the light radiance
	return distanceAttenuation * angularAttenuation * vec3(uBaseLights[lightIndex].color) * uBaseLights[lightIndex].intensity;
}


/* Calculates the color of the current fragment point with all the lights
 * of the scene and irradiance map */
vec3 calculateLighting(vec3 position, vec3 normal, vec3 albedo, float metallic, float roughness, float ao, vec4 shadows)
{
	vec3 totalLightColor = vec3(0.0);

	vec3 normalDirection	= normalize(normal);
	vec3 viewDirection		= normalize(uViewPosition - position);
	vec3 reflectivity		= mix(BASE_REFLECTIVITY, albedo, metallic);
	float normalDotView		= max(dot(normalDirection, viewDirection), 0.0);

	// ---- Calculate Direct lighting ----
	for (uint i = 0u; i < uNumLights; ++i) {
		// Calculate the light direction and distance from the current point
		vec3 lightDirection1 = -uBaseLights[i].direction;
		float lightDistance1 = -1.0;

		vec3 lightDirection2 = uBaseLights[i].position - position;
		float lightDistance2 = length(lightDirection2);
		lightDirection2 /= lightDistance2;

		bool isDirectional = uBaseLights[i].type == DIRECTIONAL_LIGHT;
		vec3 lightDirection = (lightDirection1 * float(isDirectional)) + (lightDirection2 * float(!isDirectional));
		float lightDistance = (lightDistance1 * float(isDirectional)) + (lightDistance2 * float(!isDirectional));
		if (isDirectional || (lightDistance < uBaseLights[i].range)) {
			// Calculate the light radiance
			vec3 radiance = calculateRadiance(i, lightDirection, lightDistance);

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
			vec3 currentLightColor = (diffuseRatio * diffuseColor + specularColor) * radiance * normalDotLight;

			// Calculate shadow mapping
			float shadow = calculateShadow(i, shadows);
			bool isShadowLight = uBaseLights[i].shadowIndices >= 0;
			totalLightColor += (float(isShadowLight) * (1.0 - shadow) + float(!isShadowLight)) * currentLightColor;
		}
	}

	// ---- Calculate Indirect lighting ----
	vec3 f = fresnelSchlick(normalDotView, reflectivity);

	// Calculate the ambient lighting due to IBL
	vec3 irradiance = texture(uIrradianceMap, normalDirection).rgb;
	vec3 diffuse = irradiance * albedo;

	// Calculate the specular lighting due to IBL
	vec3 reflectDirection = reflect(-viewDirection, normalDirection);
	vec3 prefilteredColor = textureLod(uPrefilterMap, reflectDirection, roughness * MAX_REFLECTION_LOD).rgb;

	vec2 environmentBRDF = texture(uBRDFMap, vec2(normalDotView, roughness)).rg;
	vec3 specular = prefilteredColor * (f * environmentBRDF.x + environmentBRDF.y);

	// Add the IBL lighting
	vec3 diffuseRatio = (1.0 - f);
	vec3 ambient = (diffuseRatio * diffuse + specular) * ao;

	return totalLightColor + ambient;
}


// ____ MAIN PROGRAM ____
void main()
{
	// Extract the data from the GBuffer
	vec2 texCoords = (0.5 * vsPosition + 0.5).xy;

	vec3 position = texture(uPosition, texCoords).rgb;
	vec3 normal = texture(uNormal, texCoords).rgb;
	vec3 albedo = texture(uAlbedo, texCoords).rgb;
	vec4 shadows = texture(uShadows, texCoords);

	vec4 material = texture(uMaterial, texCoords);
	float metallic = material.r;
	float roughness = material.g;
	float surfaceAO = material.b;

	vec3 emissive = texture(uEmissive, texCoords).rgb;

	// Calculate the output color
	vec3 color = calculateLighting(position, normal, albedo, metallic, roughness, surfaceAO, shadows);
	color += emissive;
	oColor = vec4(color, 1.0);

	// Calculate the output bright color
	bool isBright = dot(oColor.rgb, vec3(0.2126, 0.7152, 0.0722)) > 1.0;
	oBright = oColor * float(isBright) + vec4(0.0, 0.0, 0.0, 1.0) * float(!isBright);
}
