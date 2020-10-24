#version 330 core
#extension GL_ARB_shader_image_load_store : enable

// ____ GLOBAL VARIABLES ____
// Uniform variables
uniform int uMaxSize;
layout (r32ui) uniform uimage3D uImage3D;


// ____ MAIN PROGRAM ____
void main()
{
	ivec3 texCoords = ivec3(int(gl_FragCoord.x), int(gl_FragCoord.y), int(uMaxSize * gl_FragCoord.z));
	imageStore(uImage3D, texCoords, uvec4(0));
}
