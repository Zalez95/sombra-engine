# Sets all the dependencies of the project in the variables:
#   dependencies    - All the needed targets to build
#   INCLUDE_DIRS    - All the include directories
#   LIBRARIES       - All the needed libraries

find_package(Git REQUIRED)

find_package(OpenGL REQUIRED)

find_package(GLM "0.9.8" QUIET)
if(GLM_FOUND)
	message("GLM FOUND: Using system library")
else()
	message("GLM NOT FOUND: Downloading and building GLM from source")
	include(ExternalGLM)
	list(APPEND dependencies glm)
endif()

find_package(GLEW "1.11.0" QUIET)
if(GLEW_FOUND)
	message("GLEW FOUND: Using system library")
else()
	message("GLEW NOT FOUND: Downloading and building GLEW from source")
	include(ExternalGLEW)
	list(APPEND dependencies glew)
endif()

find_package(GLFW "3.2.1" QUIET)
if(GLFW_FOUND)
	message("GLFW FOUND: Using system library")
else()
	message("GLFW NOT FOUND: Downloading and building GLFW from source")
	include(ExternalGLFW)
	list(APPEND dependencies glfw)
endif()

find_package(SOIL QUIET)
if(SOIL_FOUND)
	message("SOIL FOUND: Using system library")
else()
	message("SOIL NOT FOUND: Downloading and building SOIL from source")
	include(ExternalSOIL)
	list(APPEND dependencies soil)
endif()

list(APPEND INCLUDE_DIRS ${OPENGL_INCLUDE_DIRS} ${GLM_INCLUDE_DIR} ${GLEW_INCLUDE_DIR} ${GLFW_INCLUDE_DIR} ${SOIL_INCLUDE_DIR})
list(APPEND LIBRARIES ${OPENGL_LIBRARIES} ${GLEW_LIBRARY} ${GLFW_LIBRARY} ${SOIL_LIBRARY})
