#version 330 core

// ____ CONSTANTS ____
const int MAX_POINT_LIGHTS = 4;


// ____ GLOBAL VARIABLES ____
// Input data
layout (location = 0) in vec3 aVertexPosition;			// Position attribute
layout (location = 1) in vec3 aVertexNormal;			// Normal attribute
layout (location = 2) in vec3 aVertexTangent;			// Tangent attribute
layout (location = 3) in vec2 aVertexTexCoord0;			// Vertex Texture Coords attribute

// Uniform variables
uniform mat4 uModelMatrix;								// Model space to World space Matrix
uniform mat4 uViewMatrix;								// World space to View space Matrix
uniform mat4 uProjectionMatrix;							// View space to NDC space Matrix

uniform int uNumPointLights;							// Number of lights to process
uniform vec3 uPointLightsPositions[MAX_POINT_LIGHTS];	// PointLigths positions in world space

// Output data in tangent space
out VertexData
{
	vec3 position;
	vec2 texCoord0;
} vsVertex;

flat out int vsNumPointLights;
out vec3 vsPointLightsPositions[MAX_POINT_LIGHTS];


// Functions
void main()
{
	mat4 modelViewMatrix = uViewMatrix * uModelMatrix;

	// Calculate the TBN Matrix
	vec3 bitTangent = cross(aVertexNormal, aVertexTangent);
	vec3 T = normalize(vec3(modelViewMatrix * vec4(aVertexTangent, 0.0)));
	vec3 B = normalize(vec3(modelViewMatrix * vec4(bitTangent, 0.0)));
	vec3 N = normalize(vec3(modelViewMatrix * vec4(aVertexNormal, 0.0)));
	mat3 vsTBNMatrix = transpose(mat3(T, B, N));

	// Calculate gl_Position
	vec4 vertexView	= modelViewMatrix * vec4(aVertexPosition, 1.0);
	gl_Position		= uProjectionMatrix * vertexView;

	// Calculate the Vertex data for the fragment shader in tangent space
	vsVertex.position	= vsTBNMatrix * vec3(vertexView);
	vsVertex.texCoord0	= aVertexTexCoord0;

	// Calculate the PointLights coordinates in tangent space
	vsNumPointLights = (uNumPointLights > MAX_POINT_LIGHTS)? MAX_POINT_LIGHTS : uNumPointLights;
	for (int i = 0; i < vsNumPointLights; ++i) {
		vsPointLightsPositions[i] = vsTBNMatrix * vec3(uViewMatrix * vec4(uPointLightsPositions[i], 1.0));
	}
}
