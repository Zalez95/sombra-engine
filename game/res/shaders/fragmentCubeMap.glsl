#version 330 core

// ____ GLOBAL VARIABLES ____
// Input data in world space from the vertex shader
in vec3 vsPosition;

// Uniform variables
uniform samplerCube uCubeMap;

// Output data
out vec4 glFragColor;


// ____ MAIN PROGRAM ____
void main()
{
	vec3 textureColor = texture(uCubeMap, vsPosition).rgb;
	glFragColor = vec4(textureColor, 1.0);
}
