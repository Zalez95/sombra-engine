#version 330 core

// ____ GLOBAL VARIABLES ____
// Input data
layout (location = 0) in vec3 aVertexPosition;		// Position attribute
layout (location = 3) in vec2 aVertexTexCoord0;		// Vertex Texture Coords attribute

// Uniform variables
uniform mat4 uModelMatrix;							// Model space to World space Matrix

// Output data in world space
out GeometryIn
{
	vec2 texCoord0;
} gsVertex;


// Functions
void main()
{
	gl_Position = uModelMatrix * vec4(aVertexPosition, 1.0);
	gsVertex.texCoord0 = aVertexTexCoord0;
}
