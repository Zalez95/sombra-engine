#version 330 core

// ____ GLOBAL VARIABLES ____
// Input data
layout (location = 0) in vec2 aXZPosition;				// XZ Position attribute
layout (location = 1) in vec2 aXZLocation;				// XZ patch location attribute
layout (location = 2) in int aLod;						// XZ lod attribute

// Uniform variables
uniform float uXZSize;
uniform float uMaxHeight;
uniform sampler2D uHeightMap;	// Map in object space that contains the terrain heights
uniform sampler2D uNormalMap;	// Map in object space that contains the terrain normals

// Output data in local space
out GeometryIn
{
	vec2 texCoord0;
	vec3 normal;
} gsVertex;


// ____ MAIN PROGRAM ____
void main()
{
	float xzSize = uXZSize / pow(2, aLod);
	vec2 position2D = aXZPosition * xzSize + aXZLocation;
	vec2 texCoord0 = position2D / uXZSize + 0.5;
	float height = (2.0 * texture(uHeightMap, texCoord0).r - 1.0) * uMaxHeight;

	vec3 normal = 2.0 * texture(uNormalMap, texCoord0).rgb - vec3(1.0);
	normal.y /= uMaxHeight;
	normal = normalize(normal);

	// Calculate the Vertex data for the geometry shader in local space
	gl_Position = vec4(position2D.x, height, position2D.y, 1.0);
	gsVertex.texCoord0 = texCoord0;
	gsVertex.normal = normal;
}
