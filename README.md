## Game Engine made in C++

Requirements:
- Git
- CMake
- A compiler with C++17 support
- Linux only: libgl1-mesa-dev, libasound2-dev and xorg-dev packages.

Third party libraries used:
- OpenGL			- The OpenGL library.
- GLM				- OpenGL Mathematics Library (http://glm.g-truc.net/0.9.8/index.html).
- GLEW				- The OpenGL Extension Wrangler Library (http://glew.sourceforge.net/).
- GLFW				- Library used to create windows and access to the user's input (http://www.glfw.org/).
- SOIL				- Library used to read common image file formats (http://www.lonesock.net/soil.html).
- OpenAL Soft		- Library for playing audio in 3D virtual environments (http://openal-soft.org/)
- AudioFile			- Library for reading WAV audio files (https://github.com/adamstark/AudioFile)
- Nlohmann JSON		- Library for parsing JSON Objects (https://github.com/nlohmann/json)
- Google Test		- Library used to create tests for the SombraEngine library (https://github.com/google/googletest).

Project structure:
- /build			- Object files and other CMake output
- /cmake			- CMake files used to find or download the third party libraries used in this project
- /ext				- Project dependencies source dir
- /game				- Game source dir
- /sombra-engine	- SombraEngine library source dir

Naming conventions:
- Shader extensions:
	- .vert - a vertex shader
	- .geom - a geometry shader
	- .frag - a fragment shader
	- .comp - a compute shader
- File names: the same name than the class inside it
- Types name: CamelCase
- Function and variable names: camelCase
