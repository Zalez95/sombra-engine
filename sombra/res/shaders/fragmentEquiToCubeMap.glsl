#version 330 core

// ____ CONSTANTS ____
const vec2 INV_ATAN = vec2(0.1591, 0.3183);


// ____ GLOBAL VARIABLES ____
// Input data in world space from the vertex shader
in vec3 vsPosition;

// Uniform variables
uniform sampler2D uEquirectangularMap;

// Output data
out vec4 glFragColor;


// ____ FUNCTION DEFINITIONS ____
vec2 sampleSphericalMap(vec3 v)
{
	return vec2(atan(v.z, v.x), asin(v.y)) * INV_ATAN + 0.5;
}


// ____ MAIN PROGRAM ____
void main()
{
	vec3 textureColor = texture(uEquirectangularMap, sampleSphericalMap(normalize(vsPosition))).rgb;
	glFragColor = vec4(textureColor, 1.0);
}
