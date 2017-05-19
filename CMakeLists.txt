cmake_minimum_required(VERSION 2.8)
project(FazeEngine)

# Version number
set(Test_VERSION_MAJOR 0)
set(Test_VERSION_MINOR 5)


# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib)


# Compiler flags
set(CMAKE_CXX_STANDARD 14)


# Find the external libraries needed by the exectuable
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

find_package(GLM REQUIRED)
if(GLM_FOUND)
    include_directories("${GLM_INCLUDE_DIRS}")
endif(GLM_FOUND)

find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
    include_directories("${OPENGL_INCLUDE_DIR}")
	set(LIBS ${LIBS} "${OPENGL_LIBRARY}")
endif(OPENGL_FOUND)

find_package(GLEW REQUIRED)
if(GLEW_FOUND)
    include_directories("${GLEW_INCLUDE_DIRS}")
	set(LIBS ${LIBS} "${GLEW_LIBRARIES}")
endif(GLEW_FOUND)

find_package(GLFW3 REQUIRED)
if(GLFW3_FOUND)
    include_directories("${GLFW3_INCLUDE_DIRS}")
	set(LIBS ${LIBS} "${GLFW3_LIBRARIES}")
endif(GLFW3_FOUND)

find_package(FreeImage REQUIRED)
if (FreeImage_FOUND)
    include_directories("${FreeImage_INCLUDE_DIRS}")
	set(LIBS ${LIBS} "${FreeImage_LIBRARIES}")
endif (FreeImage_FOUND)


# Copy the resource files to the output directory
file(COPY "${CMAKE_HOME_DIRECTORY}/res" DESTINATION "${CMAKE_HOME_DIRECTORY}/bin")


# Create the executable
file(GLOB_RECURSE FazeEngine_SOURCES "src/*.cpp")
file(GLOB_RECURSE FazeEngine_HEADERS "src/*.h")
add_executable(FazeEngine ${FazeEngine_HEADERS} ${FazeEngine_SOURCES})
target_link_libraries(FazeEngine ${LIBS})
