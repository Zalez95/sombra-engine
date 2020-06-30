#version 330 core

// ____ GLOBAL VARIABLES ____
// Input data
layout (location = 0) in vec3 aVertexPosition;		// Position attribute

// Uniform variables
uniform mat4 uModelMatrix;							// Model space to World space Matrix
uniform mat4 uViewMatrix;							// World space to View space Matrix
uniform mat4 uProjectionMatrix;						// View space to NDC space Matrix

uniform float uMipMapLevel;

// Output data in tangent space
out FragmentIn
{
	vec3 texCoords;
} fsVertex;


// ____ MAIN PROGRAM ____
void main()
{
	// Calculate the Model-View matrix
	mat4 modelViewMatrix = uViewMatrix * uModelMatrix;

	ivec3 voxelIndices;
	int maxSize = int(pow(2.0, uMipMapLevel));
	voxelIndices.x = gl_InstanceID % maxSize;
	voxelIndices.y = (gl_InstanceID / maxSize) % maxSize;
	voxelIndices.z = gl_InstanceID / (maxSize * maxSize);
	vec3 texCoords = vec3(voxelIndices) / float(maxSize);

	// Calculate gl_Position
	vec3 vertexLocal = texCoords + aVertexPosition / float(maxSize);
	vec4 vertexView = modelViewMatrix * vec4(vertexLocal, 1.0);
	gl_Position = uProjectionMatrix * vertexView;

	// Calculate the Vertex data for the fragment shader in tangent space
	fsVertex.texCoords = texCoords;
}
