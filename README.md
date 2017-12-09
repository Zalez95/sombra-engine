Game Engine made in C++

Third party libraries used:
- OpenGL	- The OpenGL library
- GLM		- OpenGL mathematics library (http://glm.g-truc.net/0.9.8/index.html)
- GLEW		- The OpenGL Extension Wrangler Library (http://glew.sourceforge.net/)
- GLFW		- Library used to create windows and access to the user's input (http://www.glfw.org/)
- SOIL		- Library used to support common image formats (http://www.lonesock.net/soil.html)

Project structure:
- /bin		- output binaries
- /build	- object files and other CMake output
- /cmake	- CMake files used to find the third party libraries used in this project
- /ext		- project dependencies (source code)
- /res		- other files like images, maps, 3d models and shaders
- /src		- project source files

Naming conventions:
- Shader extensions:
	- .vert - a vertex shader
	- .geom - a geometry shader
	- .frag - a fragment shader
	- .comp - a compute shader
- File names: the same name than the class inside it
- Types name: CamelCase
- Function and variable names: camelCase
