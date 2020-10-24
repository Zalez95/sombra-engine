#version 330 core

// ____ GLOBAL VARIABLES ____
// Input data
layout (location = 0) in vec2 aPosition;	// Position attribute
layout (location = 1) in vec2 aTexCoords;	// Texture coordinates attribute
layout (location = 2) in vec4 aColor;		// Color attribute
layout (location = 3) in uint aTextureId;	// Texture id attribute

// Uniform variables
uniform mat4	uProjectionMatrix;			// View space to Projection space Matrix

// Output data
out FragmentIn
{
	vec2 texCoord0;
	vec4 color;
	flat uint textureId;
} fsVertex;


// ____ MAIN PROGRAM ____
void main()
{
	gl_Position = uProjectionMatrix * vec4(aPosition, 0.0, 1.0);
	fsVertex.texCoord0 = aTexCoords;
	fsVertex.color = aColor;
	fsVertex.textureId = aTextureId;
}
