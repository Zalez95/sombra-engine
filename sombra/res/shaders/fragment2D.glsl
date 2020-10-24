#version 330 core

// ____ CONSTANTS ____
const uint MAX_TEXTURES = 16u;


// ____ GLOBAL VARIABLES ____
// Input data from the vertex shader
in FragmentIn
{
	vec2 texCoord0;
	vec4 color;
	flat uint textureId;
} fsVertex;

// Output data
out vec4 glFragColor;						// Output color

// Uniform variables
uniform sampler2D uTextures[MAX_TEXTURES];


// ____ FUNCTION DEFINITIONS ____
/* This function fixes the error "sampler arrays indexed with non-constant
 * expressions are forbidden" */
vec4 getTextureColor(uint textureId)
{
	switch (textureId) {
		case 0u:	return texture(uTextures[0], fsVertex.texCoord0);
		case 1u:	return texture(uTextures[1], fsVertex.texCoord0);
		case 2u:	return texture(uTextures[2], fsVertex.texCoord0);
		case 3u:	return texture(uTextures[3], fsVertex.texCoord0);
		case 4u:	return texture(uTextures[4], fsVertex.texCoord0);
		case 5u:	return texture(uTextures[5], fsVertex.texCoord0);
		case 6u:	return texture(uTextures[6], fsVertex.texCoord0);
		case 7u:	return texture(uTextures[7], fsVertex.texCoord0);
		case 8u:	return texture(uTextures[8], fsVertex.texCoord0);
		case 9u:	return texture(uTextures[9], fsVertex.texCoord0);
		case 10u:	return texture(uTextures[10], fsVertex.texCoord0);
		case 11u:	return texture(uTextures[11], fsVertex.texCoord0);
		case 12u:	return texture(uTextures[12], fsVertex.texCoord0);
		case 13u:	return texture(uTextures[13], fsVertex.texCoord0);
		case 14u:	return texture(uTextures[14], fsVertex.texCoord0);
		case 15u:	return texture(uTextures[15], fsVertex.texCoord0);
		default:	return vec4(0.0);
	}
}


// ____ MAIN PROGRAM ____
void main()
{
	glFragColor = fsVertex.color;

	if (fsVertex.textureId < MAX_TEXTURES) {
		glFragColor *= getTextureColor(fsVertex.textureId);
	}
}
