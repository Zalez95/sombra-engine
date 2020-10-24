#version 330 core

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


// ____ GLOBAL VARIABLES ____
// Input data in world space from the vertex shader
in FragmentIn
{
	vec3 position;
	vec2 texCoord0;
	mat3 tbn;
} fsVertex;

// Uniform variables
uniform Material uMaterial;

// Output data
layout (location = 0) out vec3 oPosition;
layout (location = 1) out vec3 oNormal;
layout (location = 2) out vec3 oAlbedo;
layout (location = 3) out vec3 oMaterial;
layout (location = 4) out vec3 oEmissive;


// ____ MAIN PROGRAM ____
void main()
{
	// Check alpha cutoff
	vec4 surfaceColor = (uMaterial.pbrMetallicRoughness.useBaseColorTexture)?
		pow(texture(uMaterial.pbrMetallicRoughness.baseColorTexture, fsVertex.texCoord0), vec4(2.2)) :
		vec4(1.0);
	surfaceColor *= uMaterial.pbrMetallicRoughness.baseColorFactor;
	if (uMaterial.checkAlphaCutoff && (surfaceColor.a < uMaterial.alphaCutoff)) {
		discard;
	}

	oPosition = fsVertex.position;

	vec3 tangentSpaceNormal = (uMaterial.useNormalTexture)?
		normalize(2.0 * texture(uMaterial.normalTexture, fsVertex.texCoord0).rgb - 1.0)
			* vec3(uMaterial.normalScale, uMaterial.normalScale, 1.0) :
		vec3(0.0, 0.0, 1.0);
	oNormal = fsVertex.tbn * tangentSpaceNormal;

	oAlbedo = surfaceColor.rgb;

	vec4 metallicRoughnessColor = texture(uMaterial.pbrMetallicRoughness.metallicRoughnessTexture, fsVertex.texCoord0);
	oMaterial.r = (uMaterial.pbrMetallicRoughness.useMetallicRoughnessTexture)?
		uMaterial.pbrMetallicRoughness.metallicFactor * metallicRoughnessColor.b :
		uMaterial.pbrMetallicRoughness.metallicFactor;
	oMaterial.g = (uMaterial.pbrMetallicRoughness.useMetallicRoughnessTexture)?
		uMaterial.pbrMetallicRoughness.roughnessFactor * metallicRoughnessColor.g :
		uMaterial.pbrMetallicRoughness.roughnessFactor;
	oMaterial.b = (uMaterial.useOcclusionTexture)?
		mix(1.0, texture(uMaterial.occlusionTexture, fsVertex.texCoord0).r, uMaterial.occlusionStrength) :
		1.0;

	oEmissive = (uMaterial.useEmissiveTexture)?
		pow(texture(uMaterial.emissiveTexture, fsVertex.texCoord0), vec4(2.2)).rgb :
		vec3(1.0);
	oEmissive *= uMaterial.emissiveFactor;
}
