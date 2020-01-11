#version 330 core

// Input data
layout (location = 0) in vec2 aVertexPosition;		// Position attribute

// Output data
out vec2		vsUV;								// Vertex UV Coordinates for the Fragment Shader

// Uniform variables
uniform mat4	uModelViewMatrix;					// Model space to View space Matrix
uniform mat4	uProjectionMatrix;					// View space to Projection space Matrix

// Functions
void main()
{
	gl_Position = uProjectionMatrix * uModelViewMatrix * vec4(aVertexPosition, 0.0, 1.0);
	vsUV = aVertexPosition;
}
