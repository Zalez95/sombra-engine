#version 330 core

// Input data
layout (location = 0) in vec2 aVertexPosition;		// Position attribute

// Output data
out vec2		vsUV;								// Vertex UV Coordinates for the Fragment Shader

// Uniform variables
uniform mat4	uModelMatrix;						// Model space to World space Matrix

// Functions
void main()
{
	gl_Position = uModelMatrix * vec4(aVertexPosition, 0.0, 1.0);
	vsUV = vec2(
		(aVertexPosition.x + 1.0) / 2.0,
		(aVertexPosition.y + 1.0) / 2.0
	);
}
