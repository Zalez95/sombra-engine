#version 330 core

// ____ GLOBAL VARIABLES ____
// Input data
layout (location = 0) in vec3 aVertexPosition;		// Position attribute
layout (location = 1) in vec3 aVertexNormal;		// Normal attribute
layout (location = 2) in vec3 aVertexTangent;		// Tangent attribute
layout (location = 3) in vec2 aVertexTexCoord0;		// Vertex Texture Coords attribute
layout (location = 4) in vec3 aLocation;			// 3D location attribute
layout (location = 5) in vec3 aVelocity;			// Velocity attribute
layout (location = 6) in float aRotation;			// Rotation attribute
layout (location = 7) in float aScale;				// Scale attribute
layout (location = 8) in float aRemainingTime;		// Remaining time attribute

// Uniform variables
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
	// Calculate gl_Position
	float s = sin(aRotation);
	float c = cos(aRotation);
	mat3 rotationScaleMatrix = mat3(mat2(c,-s, s, c) * aScale);

	vec3 viewRightWorld = vec3(uViewMatrix[0][0], uViewMatrix[1][0], uViewMatrix[2][0]);
	vec3 viewUpWorld = vec3(uViewMatrix[0][1], uViewMatrix[1][1], uViewMatrix[2][1]);

	vec3 vertexRS = rotationScaleMatrix * aVertexPosition;
	vec3 vertexWorld = aLocation + viewRightWorld * vertexRS.x + viewUpWorld * vertexRS.y;
	gl_Position = uProjectionMatrix * uViewMatrix * vec4(vertexWorld, 1.0);

	// Calculate the TBN matrix
	vec3 T = rotationScaleMatrix * aVertexTangent;
	T = viewRightWorld * T.x + viewUpWorld * T.y;

	vec3 N = rotationScaleMatrix * aVertexNormal;
	N = viewRightWorld * N.x + viewUpWorld * N.y;

	// Fix normalization issues so T and N are orthogonal
	T = normalize(T - dot(T, N) * N);

	// Calculate the bit-tangent vector in world space
	vec3 B = cross(N, T);

	// Calculate the TBN matrix
	mat3 tbnMatrix = mat3(T, B, N);

	// Calculate the Vertex data for the fragment shader in world space
	fsVertex.position	= vertexWorld;
	fsVertex.texCoord0	= aVertexTexCoord0;
	fsVertex.tbn		= tbnMatrix;
}
