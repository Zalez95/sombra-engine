#version 330 core

// ____ GLOBAL VARIABLES ____
// Input data in world space from the vertex shader
in vec3 vsPosition;

// Uniform variables
uniform sampler2D uHeightMap;
uniform vec2 uResolution;

// Output data
out vec4 glFragColor;


// ____ MAIN PROGRAM ____
void main()
{
	//      p0
	//      |
	// p1 - p - p3
	//      |
	//      p2
	float h = 2.0 * texture(uHeightMap, gl_FragCoord.xy / uResolution).r - 1.0;
	float h0 = 2.0 * texture(uHeightMap, (gl_FragCoord.xy + vec2( 0.0,-1.0)) / uResolution).r - 1.0;
	float h1 = 2.0 * texture(uHeightMap, (gl_FragCoord.xy + vec2(-1.0, 0.0)) / uResolution).r - 1.0;
	float h2 = 2.0 * texture(uHeightMap, (gl_FragCoord.xy + vec2( 0.0, 1.0)) / uResolution).r - 1.0;
	float h3 = 2.0 * texture(uHeightMap, (gl_FragCoord.xy + vec2( 1.0, 0.0)) / uResolution).r - 1.0;

	vec3 p = vec3(gl_FragCoord.x, h, gl_FragCoord.y);
	vec3 p0 = vec3(gl_FragCoord.x, h0, gl_FragCoord.y - 1.0);
	vec3 p1 = vec3(gl_FragCoord.x - 1.0, h1, gl_FragCoord.y);
	vec3 p2 = vec3(gl_FragCoord.x, h2, gl_FragCoord.y + 1.0);
	vec3 p3 = vec3(gl_FragCoord.x + 1.0, h3, gl_FragCoord.y);

	vec3 n0 = cross(p0 - p, p1 - p);
	vec3 n1 = cross(p1 - p, p2 - p);
	vec3 n2 = cross(p2 - p, p3 - p);
	vec3 n3 = cross(p3 - p, p0 - p);

	vec3 n = (n0 + n1 + n2 + n3) / 4.0;
	glFragColor = vec4(0.5 * n + vec3(0.5), 1.0);
}
