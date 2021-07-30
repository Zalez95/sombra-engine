#version 330 core

// ____ CONSTANTS ____
const vec3	BASE_REFLECTIVITY	= vec3(0.04);
const float MAX_REFLECTION_LOD	= 4.0;	// Maximum LOD of the prefilter map


// ____ GLOBAL VARIABLES ____
// Input data from the vertex shader
in vec3 vsPosition;

// Uniform variables
uniform vec3 uViewPosition;						// Camera position in World space

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
/* Returns the ratio of light that gets reflected over the light that gets
 * refracted*/
vec3 fresnelSchlick(float cosTheta, vec3 reflectivity)
{
	return reflectivity + (vec3(1.0) - reflectivity) * pow(1.0 - cosTheta, 5.0);
}


/* Calculates the color of the current fragment point with the irradiance map */
vec3 calculateAmbient(vec3 position, vec3 normal, vec3 albedo, float metallic, float roughness, float ao)
{
	vec3 totalLightColor = vec3(0.0);

	vec3 normalDirection	= normalize(normal);
	vec3 viewDirection		= normalize(uViewPosition - position);
	vec3 reflectivity		= mix(BASE_REFLECTIVITY, albedo, metallic);
	float normalDotView		= max(dot(normalDirection, viewDirection), 0.0);

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
	return (diffuseRatio * diffuse + specular) * ao;
}


// ____ MAIN PROGRAM ____
void main()
{
	// Extract the data from the GBuffer
	vec2 texCoords = (0.5 * vsPosition + 0.5).xy;

	vec3 position = texture(uPosition, texCoords).rgb;
	vec3 normal = texture(uNormal, texCoords).rgb;
	vec3 albedo = texture(uAlbedo, texCoords).rgb;

	vec4 material = texture(uMaterial, texCoords);
	float metallic = material.r;
	float roughness = material.g;
	float surfaceAO = material.b;

	vec3 emissive = texture(uEmissive, texCoords).rgb;

	// Calculate the output color
	vec3 color = calculateAmbient(position, normal, albedo, metallic, roughness, surfaceAO);
	color += emissive;
	oColor = vec4(color, 1.0);

	// Calculate the output bright color
	bool isBright = dot(oColor.rgb, vec3(0.2126, 0.7152, 0.0722)) > 1.0;
	oBright = oColor * float(isBright) + vec4(0.0, 0.0, 0.0, 1.0) * float(!isBright);
}
