#version 330 core

// ____ CONSTANTS ____
const int MAX_POINT_LIGHTS = 4;


// ____ GLOBAL VARIABLES ____
// Input data
layout (location = 0) in vec3 aVertexPosition;			// Position attribute
layout (location = 1) in vec3 aVertexNormal;			// Normal attribute
layout (location = 2) in vec2 aVertexTexCoord0;			// Vertex Texture Coords attribute

// Uniform variables
uniform mat4 uModelMatrix;								// Model space to World space Matrix
uniform mat4 uViewMatrix;								// World space to View space Matrix
uniform mat4 uProjectionMatrix;							// View space to NDC space Matrix

uniform int uNumPointLights;							// Number of lights to process
uniform vec3 uPointLightsPositions[MAX_POINT_LIGHTS];	// PointLigths positions in world space

// Output data in view space
out VertexData
{
	vec3 position;
	vec3 normal;
	vec2 texCoord0;
} vsVertex;

flat out int vsNumPointLights;
out vec3 vsPointLightsPositions[MAX_POINT_LIGHTS];


// Functions
void main()
{
	mat4 modelViewMatrix	= uViewMatrix * uModelMatrix;
	mat4 inverseTranspose	= transpose(transpose(modelViewMatrix));

	vec4 vertexView			= modelViewMatrix * vec4(aVertexPosition, 1.0f);
	gl_Position				= uProjectionMatrix * vertexView;

	// Calculate the Vertex data for the fragment shader in view space
	vsVertex.position	= vertexView.xyz;
	vsVertex.normal		= normalize(inverseTranspose * vec4(aVertexNormal, 0.0f)).xyz;
	vsVertex.texCoord0	= aVertexTexCoord0;

	// Calculate the PointLights coordinates in view space
	vsNumPointLights = (uNumPointLights > MAX_POINT_LIGHTS)? MAX_POINT_LIGHTS : uNumPointLights;
	for (int i = 0; i < vsNumPointLights; ++i) {
		vsPointLightsPositions[i] = (uViewMatrix * vec4(uPointLightsPositions[i], 1.0f)).xyz;
	}
}
