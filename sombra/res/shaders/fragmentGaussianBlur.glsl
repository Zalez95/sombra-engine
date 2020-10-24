#version 330 core

// ____ CONSTANTS ____
const float WEIGHTS[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);


// ____ GLOBAL VARIABLES ____
// Input data in world space from the vertex shader
in vec3 vsPosition;

// Uniform variables
uniform bool uHorizontal;
uniform sampler2D uColor;

// Output data
out vec4 glFragColor;


// ____ MAIN PROGRAM ____
void main()
{
	vec2 texCoords = (0.5 * vsPosition + 0.5).xy;
	vec2 texOffset = 1.0 / textureSize(uColor, 0);

	vec3 color = vec3(0.0);

	if (uHorizontal) {
		for (int i = 0; i < 5; ++i) {
			color += texture(uColor, texCoords + vec2(texOffset.x * i, 0.0)).rgb * WEIGHTS[i];
			color += texture(uColor, texCoords - vec2(texOffset.x * i, 0.0)).rgb * WEIGHTS[i];
		}
	}
	else {
		for (int i = 0; i < 5; ++i) {
			color += texture(uColor, texCoords + vec2(0.0, texOffset.y * i)).rgb * WEIGHTS[i];
			color += texture(uColor, texCoords - vec2(0.0, texOffset.y * i)).rgb * WEIGHTS[i];
		}
	}

	glFragColor = vec4(color, 1.0);
}
