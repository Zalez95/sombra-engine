#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;


// ____ CONSTANTS ____
const uint MAX_LIGHTS = 4u;


// ____ GLOBAL VARIABLES ____
// Input data in world space from the vertex shader
in GeometryIn
{
	vec2 texCoord0;
	vec3 normal;
} gsVertex[];

// Uniform variables
uniform mat4 uModelMatrix;							// Model space to World space Matrix
uniform mat4 uViewMatrix;							// World space to View space Matrix
uniform mat4 uProjectionMatrix;						// View space to NDC space Matrix

uniform uint uNumLights;							// Number of lights to process
uniform vec3 uLightsPositions[MAX_LIGHTS];			// Lights positions in world space
uniform vec3 uLightsDirections[MAX_LIGHTS];			// Lights directions in world space

// Output data in view space
out FragmentIn
{
	vec3 position;
	vec2 texCoord0;
} fsVertex;

flat out uint fsNumLights;
out vec3 fsLightsPositions[MAX_LIGHTS];
out vec3 fsLightsDirections[MAX_LIGHTS];


// ____ FUNCTION DEFINITIONS ____
/** Calculates the tangent vector of the given triangle */
vec3 calculateTangent(vec3 p0, vec3 p1, vec3 p2)
{
	vec3 v1 = p1 - p0;
	vec3 v2 = p2 - p0;
	vec2 dUV1 = gsVertex[1].texCoord0 - gsVertex[0].texCoord0;
	vec2 dUV2 = gsVertex[2].texCoord0 - gsVertex[0].texCoord0;

	float r = 1.0 / (dUV1.x * dUV2.y - dUV1.y * dUV2.x);
	return normalize(r * (v1 * dUV2.y - v2 * dUV1.y));
}


// ____ MAIN PROGRAM ____
void main()
{
	// Calculate the Model-View matrix
	mat4 modelViewMatrix = uViewMatrix * uModelMatrix;

	// Calculate the tangent vector in view space
	vec3 T = calculateTangent(vec3(gl_in[0].gl_Position), vec3(gl_in[1].gl_Position), vec3(gl_in[2].gl_Position));
	T = normalize(vec3(modelViewMatrix * vec4(T, 0.0)));

	fsNumLights = (uNumLights > MAX_LIGHTS)? MAX_LIGHTS : uNumLights;
	for (uint i = 0u; i < 3u; ++i) {
		// Calculate the normal vector in view space
		vec3 N = normalize(vec3(modelViewMatrix * vec4(gsVertex[i].normal, 0.0)));

		// Fix normalization issues so T and N are orthogonal
		vec3 T2 = normalize(T - dot(T, N) * N);

		// Calculate the bit-tangent vector in view space
		vec3 B = cross(N, T2);

		// Calculate the TBN matrix
		mat3 tbnMatrix = transpose(mat3(T2, B, N));

		// Calculate gl_Position
		vec4 vertexView	= modelViewMatrix * gl_in[i].gl_Position;
		gl_Position		= uProjectionMatrix * vertexView;

		// Calculate the Vertex data for the fragment shader in tangent space
		fsVertex.position	= tbnMatrix * vec3(vertexView);
		fsVertex.texCoord0	= gsVertex[i].texCoord0;

		// Calculate the Lights coordinates in tangent space
		for (uint j = 0u; j < fsNumLights; ++j) {
			fsLightsPositions[j] = tbnMatrix * vec3(uViewMatrix * vec4(uLightsPositions[j], 1.0));
			fsLightsDirections[j] = tbnMatrix * vec3(uViewMatrix * vec4(uLightsDirections[j], 0.0));
		}

		EmitVertex();
	}

	EndPrimitive();
}
