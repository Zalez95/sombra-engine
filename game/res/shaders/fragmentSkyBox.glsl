#version 330 core

// ____ GLOBAL VARIABLES ____
// Input data in world space from the vertex shader
in vec3 vsPosition;

// Uniform variables
uniform samplerCube uCubeMap;

// Output data
layout (location = 0) out vec4 oColor;
layout (location = 1) out vec4 oBright;


// ____ MAIN PROGRAM ____
void main()
{
	vec3 textureColor = texture(uCubeMap, vsPosition).rgb;
	oColor = vec4(textureColor, 1.0);

	// Calculate the output bright color
	bool isBright = dot(oColor.rgb, vec3(0.2126, 0.7152, 0.0722)) > 1.0;
	oBright = oColor * float(isBright) + vec4(0.0, 0.0, 0.0, 1.0) * float(!isBright);
}
