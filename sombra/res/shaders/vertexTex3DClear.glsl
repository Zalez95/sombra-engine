#version 330 core

// ____ GLOBAL VARIABLES ____
// Input data
layout (location = 0) in vec2 aVertexPosition;		// Position attribute

// Uniform variables
uniform int uMaxSize;


// ____ MAIN PROGRAM ____
void main()
{
	// Calculate gl_Position
	vec3 projectedPoint = 2.0 * vec3(aVertexPosition, float(gl_InstanceID) / float(uMaxSize)) - vec3(1.0);
	gl_Position = vec4(projectedPoint, 1.0);
}
