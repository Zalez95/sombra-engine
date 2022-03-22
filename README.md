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
- GLFW				- Library used for creating windows and access to the user's input (http://www.glfw.org/).
- STB				- Library used for reading common image file formats (http://www.lonesock.net/soil.html).
- MiniAudio		    - Library for playing audio in 3D virtual environments (https://github.com/mackron/miniaudio)
- Nlohmann JSON		- Library for parsing JSON Objects (https://github.com/nlohmann/json)
- FreeType			- Library for reading OpenType font files (https://www.freetype.org)
- Google Test		- Library used for creating unit tests for the SombraEngine library (https://github.com/google/googletest).
- Dear ImGui		- Library used for creating the GUI of the SombraEngine editor (https://github.com/ocornut/imgui).

Project structure:
- /build			- Object files and other CMake output
- /editor			- SombraEngine Editor
- /example			- Game example created with the SombraEngine
- /sombra			- SombraEngine library source dir
