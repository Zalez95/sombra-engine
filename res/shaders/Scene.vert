#version 330 core

// ____ CONSTANTS ____
const int MAX_POINT_LIGHTS = 4;


// ____ GLOBAL VARIABLES ____
// Input data
layout (location = 0) in vec3 a_VertexPosition;			// Position attribute
layout (location = 1) in vec3 a_VertexNormal;			// Normal attribute
layout (location = 2) in vec2 a_VertexUV;				// Vertex UV Coords attribute

// Uniform variables
uniform mat4 u_ModelViewMatrix;							// Model space to View space Matrix
uniform mat4 u_ProjectionMatrix;						// View space to Perspective space Matrix

uniform int u_NumPointLights;							// Number of lights to process
uniform vec3 u_PointLightsPositions[MAX_POINT_LIGHTS];	// PointLigths positions in world space

// Output data in view space
out VertexData {
	vec3 mPosition;
	vec3 mNormal;
	vec2 mUV;
} vs_Vertex;

flat out int vs_NumPointLights;
out vec3 vs_PointLightsPositions[MAX_POINT_LIGHTS];


// Functions
void main()
{
	vec4 vertexView			= u_ModelViewMatrix * vec4(a_VertexPosition, 1.0f);
	mat4 inverseTranspose	= transpose(inverse(u_ModelViewMatrix));
	gl_Position				= u_ProjectionMatrix * vertexView;

	// Calculate the Vertex data for the fragment shader in view space
	vs_Vertex.mPosition		= vertexView.xyz;
	vs_Vertex.mNormal		= normalize(inverseTranspose * vec4(a_VertexNormal, 0.0f)).xyz;
	vs_Vertex.mUV			= a_VertexUV;

	// Calculate the PointLights coordinates in view space
	vs_NumPointLights = (u_NumPointLights > MAX_POINT_LIGHTS)? MAX_POINT_LIGHTS : u_NumPointLights;
	for (int i = 0; i < vs_NumPointLights; ++i) {
		vs_PointLightsPositions[i] = (u_ModelViewMatrix * vec4(u_PointLightsPositions[i], 1.0f)).xyz;
	}
}
