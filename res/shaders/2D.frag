#version 330 core

// Input data from the vertex shader
in vec2			fin_UV;			// The Vertex UV Coordinates from the Vertex Shader

// Output data
out vec4		gl_FragColor;		// Output color

// Uniform variables
uniform sampler2D u_TextureSampler;


// Functions
void main()
{
	gl_FragColor = texture(u_TextureSampler, fin_UV);
}
