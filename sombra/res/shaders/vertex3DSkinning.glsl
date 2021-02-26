#version 330 core

// ____ CONSTANTS ____
const uint MAX_JOINTS = 84u;


// ____ GLOBAL VARIABLES ____
// Input data
layout (location = 0) in vec3 aVertexPosition;		// Position attribute
layout (location = 6) in uvec4 aVertexJoints;		// Vertex joint indices
layout (location = 7) in vec4 aVertexWeights;		// Vertex weights of each joint

// Uniform variables
uniform mat4 uModelMatrix;							// Model space to World space Matrix
uniform mat4 uViewMatrix;							// World space to View space Matrix
uniform mat4 uProjectionMatrix;						// View space to NDC space Matrix

uniform mat3x4 uJointMatrices[MAX_JOINTS];			// The joint matrices of the Skeleton

// Output data
out vec3 vsPosition;								// Global Vertex position for the Fragment Shader


// Functions
void main()
{
	// Calculate the skeleton pose matrix
	mat3x4 poseMat3x4 = aVertexWeights.x * uJointMatrices[aVertexJoints.x]
					+ aVertexWeights.y * uJointMatrices[aVertexJoints.y]
					+ aVertexWeights.z * uJointMatrices[aVertexJoints.z]
					+ aVertexWeights.w * uJointMatrices[aVertexJoints.w];
	mat4 poseMatrix = mat4(poseMat3x4[0], poseMat3x4[1], poseMat3x4[2], vec4(0.0, 0.0, 0.0, 1.0));
	poseMatrix = transpose(poseMatrix);

	// Calculate the model matrix with the pose
	mat4 modelPoseMatrix = uModelMatrix * poseMatrix;

	// Calculate gl_Position
	vec4 vertexWorld = modelPoseMatrix * vec4(aVertexPosition, 1.0);
	gl_Position = uProjectionMatrix * uViewMatrix * vertexWorld;

	vsPosition = vec3(vertexWorld);
}
