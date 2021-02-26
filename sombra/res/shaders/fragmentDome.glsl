#version 330 core

// ____ CONSTANTS ____
const vec3 BASE_COLOR = vec3(0.18, 0.27, 0.47);
const float DOME_RADIUS = 1000.0;


// ____ GLOBAL VARIABLES ____
// Input data in world space from the vertex shader
in vec3 vsPosition;

// Output data
out vec4 glFragColor;


// ____ MAIN PROGRAM ____
void main()
{
	float r = -0.00025 * (vsPosition.y - DOME_RADIUS) + BASE_COLOR.r;
	float g = -0.00022 * (vsPosition.y - DOME_RADIUS) + BASE_COLOR.g;
	float b = -0.00019 * (vsPosition.y - DOME_RADIUS) + BASE_COLOR.b;

	glFragColor = vec4(r, g, b, 1.0);
}
