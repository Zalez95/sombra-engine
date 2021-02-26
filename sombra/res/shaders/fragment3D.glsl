#version 330 core

// ____ GLOBAL VARIABLES ____
// Input data in world space from the vertex shader
in vec3 vsPosition;

// Uniform variables
uniform vec4 uColor;

// Output data
layout (location = 0) out vec4 oColor;
layout (location = 1) out vec4 oBright;


// ____ MAIN PROGRAM ____
void main()
{
	oColor = uColor;
	oBright = vec4(0.0);
}
