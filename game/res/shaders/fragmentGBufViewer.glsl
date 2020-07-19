#version 330 core

// ____ GLOBAL VARIABLES ____
// Input data in tangent space from the vertex shader
in vec3 vsPosition;

uniform sampler2D uPosition;
uniform sampler2D uNormal;
uniform sampler2D uAlbedo;
uniform sampler2D uMaterial;

// Output data
out vec4 glFragColor;


// ____ MAIN PROGRAM ____
void main()
{
	glFragColor = ((vsPosition.x < 0) && (vsPosition.y < 0))? texture(uPosition, vsPosition.xy + vec2(1.0)) :
		((vsPosition.x >= 0) && (vsPosition.y < 0))? texture(uNormal, vec2(vsPosition.x, vsPosition.y + 1.0)) :
		((vsPosition.x < 0) && (vsPosition.y >= 0))? texture(uAlbedo, vec2(vsPosition.x + 1.0, vsPosition.y)) :
		((vsPosition.x >= 0) && (vsPosition.y >= 0))? texture(uMaterial, vsPosition.xy) :
		vec4(0,0,0,1);
}
