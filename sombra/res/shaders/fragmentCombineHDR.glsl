#version 330 core

// ____ CONSTANTS ____
const float	SCREEN_GAMMA		= 2.2;	// Monitor is in sRGB color space


// ____ GLOBAL VARIABLES ____
// Input data in world space from the vertex shader
in vec3 vsPosition;

// Uniform variables
uniform sampler2D uColor0;
uniform sampler2D uColor1;

// Output data
out vec4 glFragColor;


// ____ MAIN PROGRAM ____
void main()
{
	vec2 texCoords = (0.5 * vsPosition + 0.5).xy;
	vec3 color = texture(uColor0, texCoords).rgb;
	color += texture(uColor1, texCoords).rgb;

	// Gamma correction
	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0 / SCREEN_GAMMA));

	glFragColor = vec4(color, 1.0);
}
//TODO:shadow?