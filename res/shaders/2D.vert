#version 130

// Input data
in vec2 a_VertexPosition;		// Position attribute

// Output data
out vec2		fin_UV;								// Vertex UV Coordinates for the Fragment Shader

// Uniform variables
uniform mat4	u_ModelMatrix;						// Model space to World space Matrix
uniform mat4	u_ProjectionMatrix;					// View Space to NDC space Matrix

// Functions
void main()
{
	gl_Position = u_ProjectionMatrix * u_ModelMatrix * vec4(a_VertexPosition, 0.0f, 1.0f);
	fin_UV = vec2(
		(a_VertexPosition.x + 1.0) / 2.0,
		(a_VertexPosition.y + 1.0) / 2.0
	);
}
