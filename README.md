Game Engine made in C++

Third party libraries needed:
- OpenGL    - The OpenGL library
- GLM       - OpenGL mathematics library (http://glm.g-truc.net/0.9.8/index.html)
- GLEW      - The OpenGL Extension Wrangler Library (http://glew.sourceforge.net/)
- GLFW      - Library used to create windows and access to the user's input (http://www.glfw.org/)
- FreeImage - Library used to support common image formats (http://freeimage.sourceforge.net/)

Project structure:
- /bin		- output binaries
- /build	- object files and other CMake output
- /cmake    - CMake files used to find the third party libraries used in the project
- /src		- app source files and projects
- /res      - other files and like images, shaders and 3d models

Naming conventions:
- Shader extensions:
	- .vert - a vertex shader
	- .tesc - a tessellation control shader
	- .tese - a tessellation evaluation shader
	- .geom - a geometry shader
	- .frag - a fragment shader
	- .comp - a compute shader
- File names: the same name than the class inside it
- Types name: CamelCase
- Functions and variables name: camelCase
