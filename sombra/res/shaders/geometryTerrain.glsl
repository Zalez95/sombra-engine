#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

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

// Output data in world space
out FragmentIn
{
	vec3 position;
	vec2 texCoord0;
	mat3 tbn;
} fsVertex;


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
	// Calculate the tangent vector in world space
	vec3 T = calculateTangent(vec3(gl_in[0].gl_Position), vec3(gl_in[1].gl_Position), vec3(gl_in[2].gl_Position));
	T = normalize(vec3(uModelMatrix * vec4(T, 0.0)));

	for (uint i = 0u; i < 3u; ++i) {
		// Calculate the normal vector in world space
		vec3 N = normalize(vec3(uModelMatrix * vec4(gsVertex[i].normal, 0.0)));

		// Fix normalization issues so T and N are orthogonal
		vec3 T2 = normalize(T - dot(T, N) * N);

		// Calculate the bit-tangent vector in world space
		vec3 B = cross(N, T2);

		// Calculate the TBN matrix
		mat3 tbnMatrix = mat3(T2, B, N);

		// Calculate gl_Position
		vec4 vertexWorld = uModelMatrix * gl_in[i].gl_Position;
		gl_Position = uProjectionMatrix * uViewMatrix * vertexWorld;

		// Calculate the Vertex data for the fragment shader in world space
		fsVertex.position	= vec3(vertexWorld);
		fsVertex.texCoord0	= gsVertex[i].texCoord0;
		fsVertex.tbn		= tbnMatrix;

		EmitVertex();
	}

	EndPrimitive();
}
