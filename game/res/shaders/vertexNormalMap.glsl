#version 330 core

// ____ GLOBAL VARIABLES ____
// Input data
layout (location = 0) in vec3 aVertexPosition;		// Position attribute
layout (location = 1) in vec3 aVertexNormal;		// Normal attribute
layout (location = 2) in vec3 aVertexTangent;		// Tangent attribute
layout (location = 3) in vec2 aVertexTexCoord0;		// Vertex Texture Coords attribute

// Uniform variables
uniform mat4 uModelMatrix;							// Model space to World space Matrix
uniform mat4 uViewMatrix;							// World space to View space Matrix
uniform mat4 uProjectionMatrix;						// View space to NDC space Matrix

// Output data in world space
out FragmentIn
{
	vec3 position;
	vec2 texCoord0;
	mat3 tbn;
} fsVertex;


// ____ MAIN PROGRAM ____
void main()
{
	// Calculate the tangent and normal vectors in world space
	vec3 T = normalize(vec3(uModelMatrix * vec4(aVertexTangent, 0.0)));
	vec3 N = normalize(vec3(uModelMatrix * vec4(aVertexNormal, 0.0)));

	// Fix normalization issues so T and N are orthogonal
	T = normalize(T - dot(T, N) * N);

	// Calculate the bit-tangent vector in world space
	vec3 B = cross(N, T);

	// Calculate the TBN matrix
	mat3 tbnMatrix = mat3(T, B, N);

	// Calculate gl_Position
	vec4 vertexWorld = uModelMatrix * vec4(aVertexPosition, 1.0);
	gl_Position = uProjectionMatrix * uViewMatrix * vertexWorld;

	// Calculate the Vertex data for the fragment shader in world space
	fsVertex.position	= vec3(vertexWorld);
	fsVertex.texCoord0	= aVertexTexCoord0;
	fsVertex.tbn		= tbnMatrix;
}
