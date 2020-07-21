#version 330 core
#extension GL_ARB_shader_image_load_store : enable

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
// Input data space from the geometry shader
in FragmentIn
{
	vec2 texCoord0;
} fsVertex;

flat in uint fsProjectionAxis;

// Uniform variables
uniform Material uMaterial;
uniform int uMaxVoxels;
layout (r32ui) uniform uimage3D uVoxelImage;


// ____ FUNCTION DEFINITIONS ____
/** Parses the given vec4 value to a single 32 bit unsigned integer */
uint convertVec4ToR32UI(vec4 value)
{
	uint ret = 0u;
	ret |= (uint(value.x) & 0xFFu);
	ret |= (uint(value.y) & 0xFFu) << 8u;
	ret |= (uint(value.z) & 0xFFu) << 16u;
	ret |= (uint(value.w) & 0xFFu) << 24u;
	return ret;
}


/** Parses the given 32 bit unsigned integer to a vec4 */
vec4 convertR32UIToVec4(uint value)
{
	vec4 ret = vec4(0.0);
	ret.x = float(value & 0xFFu);
	ret.y = float((value >> 8u )& 0xFFu);
	ret.z = float((value >> 16u) & 0xFFu);
	ret.w = float((value >> 24u) & 0xFFu);
	return ret;
}


/** Stores the given value to the voxel texture, averaging the value with the
 * one already stored using moving averages */
void storeToVoxel(ivec3 voxelCoords, vec4 value)
{
	value.xyz *= 255.0;		// The value data must be in the range [0, 255]
	value.w = 1.0;			// The w value is used for storing the counter needed for the moving average

	uint newValue = convertVec4ToR32UI(value);
	uint oldValue = 0u;
	uint currentValue = 0u;

	while ((currentValue = imageAtomicCompSwap(uVoxelImage, voxelCoords, oldValue, newValue)) != oldValue) {
		oldValue = currentValue;
		vec4 vCurrentValue = convertR32UIToVec4(currentValue);
		vCurrentValue.xyz = vCurrentValue.xyz * vCurrentValue.w;	// Denormalize
		vec4 vCurrentValue2 = vCurrentValue + value;				// Add new value
		vCurrentValue2.xyz /= vCurrentValue2.w;						// Renormalize
		newValue = convertVec4ToR32UI(vCurrentValue2);
	}
}


// ____ MAIN PROGRAM ____
void main()
{
	// Get the texture data for the current fragment
	vec4 surfaceColor = (uMaterial.pbrMetallicRoughness.useBaseColorTexture)?
		pow(texture(uMaterial.pbrMetallicRoughness.baseColorTexture, fsVertex.texCoord0), vec4(2.2)) :
		vec4(1.0);
	surfaceColor *= uMaterial.pbrMetallicRoughness.baseColorFactor;

	// Check alpha cutoff
	if (uMaterial.checkAlphaCutoff && (surfaceColor.a < uMaterial.alphaCutoff)) {
		discard;
	}
	else {
		vec3 color = surfaceColor.rgb;

		// Store the color in the Voxel 3D texture using the fragment
		// coordinates (the viewport size is the same than the voxel grid)
		ivec3 voxelCoords = ivec3(int(gl_FragCoord.x), int(gl_FragCoord.y), int(uMaxVoxels * gl_FragCoord.z));
		voxelCoords =
			(fsProjectionAxis == 0u)? ivec3(voxelCoords.z, voxelCoords.y, uMaxVoxels - voxelCoords.x) :
			(fsProjectionAxis == 1u)? ivec3(voxelCoords.x, voxelCoords.z, uMaxVoxels - voxelCoords.y) :
			voxelCoords;
		storeToVoxel(voxelCoords, vec4(color, 0.0));
	}
}
