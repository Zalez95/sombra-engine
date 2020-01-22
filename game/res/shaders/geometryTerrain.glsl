#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;


// ____ CONSTANTS ____
const uint MAX_POINT_LIGHTS = 4u;


// ____ GLOBAL VARIABLES ____
// Input data in world space from the vertex shader
in GeometryIn
{
	vec2 texCoord0;
} gsVertex[];

// Uniform variables
uniform mat4 uViewMatrix;								// World space to View space Matrix
uniform mat4 uProjectionMatrix;							// View space to NDC space Matrix

uniform uint uNumPointLights;							// Number of lights to process
uniform vec3 uPointLightsPositions[MAX_POINT_LIGHTS];	// PointLigths positions in world space

// Output data in tangent space
out FragmentIn
{
	vec3 position;
	vec2 texCoord0;
} fsVertex;

flat out uint fsNumPointLights;
out vec3 fsPointLightsPositions[MAX_POINT_LIGHTS];


// ____ MAIN PROGRAM ____
void main() {
	// Calculate the TBN matrix (note the input data should be in view space)
	vec3 p0 = vec3(uViewMatrix * gl_in[0].gl_Position);
	vec3 p1 = vec3(uViewMatrix * gl_in[1].gl_Position);
	vec3 p2 = vec3(uViewMatrix * gl_in[2].gl_Position);

	vec3 v1 = normalize(p1 - p0);
	vec3 v2 = normalize(p2 - p0);

	vec3 N = normalize(cross(v1, v2));
	vec3 T = normalize(cross(v1, N));
	vec3 B = cross(N, T);

	mat3 tbnMatrix = transpose(mat3(T, B, N));

	// Calculate the PointLights coordinates in tangent space
	fsNumPointLights = (uNumPointLights > MAX_POINT_LIGHTS)? MAX_POINT_LIGHTS : uNumPointLights;
	for (uint i = 0u; i < fsNumPointLights; ++i) {
		fsPointLightsPositions[i] = tbnMatrix * vec3(uViewMatrix * vec4(uPointLightsPositions[i], 1.0));
	}

	// Emit the vertices in tangent space
	gl_Position = uProjectionMatrix * vec4(p0, 1.0);
	fsVertex.position = tbnMatrix * vec3(p0);
	fsVertex.texCoord0 = gsVertex[0].texCoord0;
	EmitVertex();

	gl_Position = uProjectionMatrix * vec4(p1, 1.0);
	fsVertex.position = tbnMatrix * vec3(p1);
	fsVertex.texCoord0 = gsVertex[1].texCoord0;
	EmitVertex();

	gl_Position = uProjectionMatrix * vec4(p2, 1.0);
	fsVertex.position = tbnMatrix * vec3(p2);
	fsVertex.texCoord0 = gsVertex[2].texCoord0;
	EmitVertex();

	EndPrimitive();
}
