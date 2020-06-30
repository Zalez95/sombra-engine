#version 330 core

// ____ CONSTANTS ____
const float PI = 3.14159265359;
const float SAMPLE_DELTA = 0.025;

// ____ GLOBAL VARIABLES ____
// Input data in world space from the vertex shader
in vec3 vsPosition;

// Uniform variables
uniform samplerCube uCubeMap;

// Output data
out vec4 glFragColor;


// ____ MAIN PROGRAM ____
void main()
{
	vec3 normal = normalize(vsPosition);
	vec3 right = cross(vec3(0.0, 1.0, 0.0), normal);
	vec3 up = cross(normal, right);

	// See https://learnopengl.com/PBR/IBL/Diffuse-irradiance
	vec3 irradiance = vec3(0.0);
	int numSamples = 0;
	for (float phi = 0.0; phi < 2.0 * PI; phi += SAMPLE_DELTA) {
		for (float theta = 0.0; theta < 0.5 * PI; theta += SAMPLE_DELTA) {
			// spherical to cartesian (in tangent space)
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			// tangent space to world
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

			irradiance += texture(uCubeMap, sampleVec).rgb * cos(theta) * sin(theta);
			numSamples++;
		}
	}
	irradiance = PI / float(numSamples) * irradiance;

	glFragColor = vec4(irradiance, 1.0);
}
