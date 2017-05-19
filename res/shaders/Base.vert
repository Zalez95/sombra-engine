#version 330 core

// Input data
layout (location = 0) in vec3 a_VertexPosition;		// Position attribute

// Uniform variables
uniform mat4 u_ModelMatrix;			// Model space to World space Matrix
uniform mat4 u_ViewMatrix;			// World space to View space Matrix
uniform mat4 u_ProjectionMatrix;	// View space to Perspective space Matrix

// Functions
void main()
{
	vec4 vertexWorld	= u_ModelMatrix * vec4(a_VertexPosition, 1.0f);
	gl_Position			= u_ProjectionMatrix * u_ViewMatrix * vertexWorld;
}
