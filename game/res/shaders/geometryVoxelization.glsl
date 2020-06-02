#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

// ____ GLOBAL VARIABLES ____
// Input data in world space from the vertex shader
in GeometryIn
{
	vec2 texCoord0;
} gsVertex[];

// Uniform variables
uniform mat4 uProjectionMatrices[3];	// The orthographic projection matrices for the XYZ axes

// Output data
out FragmentIn
{
	vec2 texCoord0;
} fsVertex;

flat out uint fsProjectionAxis;


// ____ MAIN PROGRAM ____
void main() {
	// Calculate the TBN matrix (note the input data should be in view space)
	vec3 p0 = vec3(gl_in[0].gl_Position);
	vec3 p1 = vec3(gl_in[1].gl_Position);
	vec3 p2 = vec3(gl_in[2].gl_Position);

	vec3 v1 = p1 - p0;
	vec3 v2 = p2 - p0;
	vec3 normal = cross(v1, v2);

	// Project towards the axis that maximices the projected area
	float projXArea = 0.5 * abs( dot(normal, vec3(1.0, 0.0, 0.0)) );
	float projYArea = 0.5 * abs( dot(normal, vec3(0.0, 1.0, 0.0)) );
	float projZArea = 0.5 * abs( dot(normal, vec3(0.0, 0.0, 1.0)) );

	if (projXArea > projYArea) {
		if (projXArea > projZArea) {
			fsProjectionAxis = 0u;
		}
		else {
			fsProjectionAxis = 2u;
		}
	}
	else {
		if (projYArea > projZArea) {
			fsProjectionAxis = 1u;
		}
		else {
			fsProjectionAxis = 2u;
		}
	}

	// Emit the points
	gl_Position = uProjectionMatrices[fsProjectionAxis] * vec4(p0, 1.0);
	fsVertex.texCoord0 = gsVertex[0].texCoord0;
	EmitVertex();

	gl_Position = uProjectionMatrices[fsProjectionAxis] * vec4(p1, 1.0);
	fsVertex.texCoord0 = gsVertex[1].texCoord0;
	EmitVertex();

	gl_Position = uProjectionMatrices[fsProjectionAxis] * vec4(p2, 1.0);
	fsVertex.texCoord0 = gsVertex[2].texCoord0;
	EmitVertex();

	EndPrimitive();
}
