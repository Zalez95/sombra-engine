#version 330 core

// ____ CONSTANTS ____
const float	SCREEN_GAMMA		= 2.2;	// Monitor is in sRGB color space


// ____ GLOBAL VARIABLES ____
// Input data in tangent space from the vertex shader
in FragmentIn
{
	vec3 texCoords;
} fsVertex;

// Uniform variables
uniform usampler3D uTexture3D;
uniform float uMipMapLevel;

// Output data
out vec4 glFragColor;


// ____ FUNCTION DEFINITIONS ____
/** Parses the given 32 bit unsigned integer to a vec4 */
vec4 convertR32UIToVec4(uint value)
{
	vec4 ret = vec4(0);
	ret.x = float(value & 0xFFu);
	ret.y = float((value >> 8u )& 0xFFu);
	ret.z = float((value >> 16u) & 0xFFu);
	ret.w = float((value >> 24u) & 0xFFu);
	return ret;
}


// ____ MAIN PROGRAM ____
void main()
{
	uint value = textureLod(uTexture3D, fsVertex.texCoords, uMipMapLevel).r;
	vec4 voxelRGBA = convertR32UIToVec4(value);

	// Check alpha cutoff
	if (voxelRGBA.w == 0.0) {
		discard;
	}
	else {
		vec3 color = voxelRGBA.rgb / 255.0;

		// Gamma correction
		color = color / (color + vec3(1.0));
		color = pow(color, vec3(1.0 / SCREEN_GAMMA));

		glFragColor = vec4(color, 1.0);
	}
}
