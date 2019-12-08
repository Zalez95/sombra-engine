#version 330 core

// ____ GLOBAL VARIABLES ____
// Input data
layout (location = 0) in vec3 aVertexPosition;			// Position attribute

// Uniform variables
uniform mat4 uViewMatrix;								// World space to View space Matrix
uniform mat4 uProjectionMatrix;							// View space to NDC space Matrix

// Output data
out vec3 vsPosition;									// Global Vertex position for the Fragment Shader


// Functions
void main()
{
	// Calculate gl_Position
	gl_Position = uProjectionMatrix * uViewMatrix * vec4(aVertexPosition, 1.0);

	vsPosition = vec3(aVertexPosition);
}
