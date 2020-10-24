#version 330 core

// ____ GLOBAL VARIABLES ____
// Input data in world space from the vertex shader
in vec3 vsPosition;

// Uniform variables
uniform sampler2D uHeightMap;
uniform vec2 uResolution;
uniform float uStrength;

// Output data
out vec4 glFragColor;


// ____ MAIN PROGRAM ____
void main()
{
	// h0 - h1 - h2
	// |    |    |
	// h3 - h  - h4
	// |    |    |
	// h5 - h6 - h7
	float h0 = texture(uHeightMap, (gl_FragCoord.xy + vec2(-1.0,-1.0)) / uResolution).r;
	float h1 = texture(uHeightMap, (gl_FragCoord.xy + vec2( 0.0,-1.0)) / uResolution).r;
	float h2 = texture(uHeightMap, (gl_FragCoord.xy + vec2( 1.0,-1.0)) / uResolution).r;
	float h3 = texture(uHeightMap, (gl_FragCoord.xy + vec2(-1.0, 0.0)) / uResolution).r;
	float h4 = texture(uHeightMap, (gl_FragCoord.xy + vec2( 1.0, 0.0)) / uResolution).r;
	float h5 = texture(uHeightMap, (gl_FragCoord.xy + vec2(-1.0, 1.0)) / uResolution).r;
	float h6 = texture(uHeightMap, (gl_FragCoord.xy + vec2( 0.0, 1.0)) / uResolution).r;
	float h7 = texture(uHeightMap, (gl_FragCoord.xy + vec2( 1.0, 1.0)) / uResolution).r;

	// Sobel filter
	vec3 normal = vec3(0.0);
	normal.x = h0 + 2.0 * h3 + h5 - h2 - 2.0 * h4 - h7;
	normal.y = h0 + 2.0 * h1 + h2 - h5 - 2.0 * h6 - h7;
	normal.z = 1 / uStrength;
	normal = normalize(normal);

	glFragColor = vec4(0.5 * normal + 0.5, 1.0);
}
