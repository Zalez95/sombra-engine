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
};

struct SplatmapMaterial
{
	int numMaterials;
	Material materials[4];
	sampler2D splatmapTexture;
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
uniform SplatmapMaterial uSMaterial;

// Output data
layout (location = 0) out vec3 oPosition;
layout (location = 1) out vec3 oNormal;
layout (location = 2) out vec3 oAlbedo;
layout (location = 3) out vec3 oMaterial;


// ____ FUNCTION DEFINITIONS ____
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
	oPosition = fsVertex.position;
	oNormal = vec3(0.0);
	oAlbedo = vec3(0.0);
	oMaterial = vec3(0.0);

	// Get the splatmap texture data for the current fragment
	vec4 splatmapColor = texture(uSMaterial.splatmapTexture, fsVertex.texCoord0);
	for (int i = 0; i < uSMaterial.numMaterials; ++i) {
		float rate = getMaterialRate(splatmapColor, i);

		vec3 tangentSpaceNormal = (uSMaterial.materials[i].useNormalTexture)?
			normalize(2.0 * texture(uSMaterial.materials[i].normalTexture, fsVertex.texCoord0).rgb - 1.0)
				* vec3(uSMaterial.materials[i].normalScale, uSMaterial.materials[i].normalScale, 1.0) :
			vec3(0.0, 0.0, 1.0);
		oNormal += rate * fsVertex.tbn * tangentSpaceNormal;

		vec4 surfaceColor = (uSMaterial.materials[i].pbrMetallicRoughness.useBaseColorTexture)?
			pow(texture(uSMaterial.materials[i].pbrMetallicRoughness.baseColorTexture, fsVertex.texCoord0), vec4(2.2)) :
			vec4(1.0);
		surfaceColor *= uSMaterial.materials[i].pbrMetallicRoughness.baseColorFactor;
		oAlbedo += rate * surfaceColor.rgb;

		vec4 metallicRoughnessColor = texture(uSMaterial.materials[i].pbrMetallicRoughness.metallicRoughnessTexture, fsVertex.texCoord0);
		float metallic = (uSMaterial.materials[i].pbrMetallicRoughness.useMetallicRoughnessTexture)?
			uSMaterial.materials[i].pbrMetallicRoughness.metallicFactor * metallicRoughnessColor.b :
			uSMaterial.materials[i].pbrMetallicRoughness.metallicFactor;
		float roughness = (uSMaterial.materials[i].pbrMetallicRoughness.useMetallicRoughnessTexture)?
			uSMaterial.materials[i].pbrMetallicRoughness.roughnessFactor * metallicRoughnessColor.g :
			uSMaterial.materials[i].pbrMetallicRoughness.roughnessFactor;
		oMaterial += rate * vec3(metallic, roughness, 1.0);
	}
}
