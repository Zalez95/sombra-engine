#version 330 core

// ____ GLOBAL VARIABLES ____
// Input data from the vertex shader
in vec2 vsUV;			// The Vertex UV Coordinates from the Vertex Shader

// Output data
out vec4 glFragColor;	// Output color

// Uniform variables
uniform sampler2D uTextureSampler;


// Functions
void main()
{
	glFragColor = texture(uTextureSampler, vsUV);
}
