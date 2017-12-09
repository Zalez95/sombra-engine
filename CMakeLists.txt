cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

###############################################################################
# 								FAZE ENGINE
###############################################################################
# Project name and version
project(FazeEngine VERSION 0.5.0 LANGUAGES CXX)

# Set the external libraries directories
set(EXTERNAL_PATH "${CMAKE_SOURCE_DIR}/ext")
set(EXTERNAL_INSTALL_LOCATION "${CMAKE_BINARY_DIR}/ext")

# Change the global variables
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(BUILD_SHARED_LIBRARIES OFF)

# Find the project header and source files
file(GLOB_RECURSE FazeEngine_HEADERS "${CMAKE_SOURCE_DIR}/src/*.h")
file(GLOB_RECURSE FazeEngine_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

# Create the executable
add_executable(FazeEngine ${FazeEngine_HEADERS} ${FazeEngine_SOURCES})
target_compile_options(FazeEngine PRIVATE -std=c++14 -Wall -Wextra -Wpedantic -Werror)

# Add the dependencies
include(Dependencies)
add_dependencies(FazeEngine ${dependencies})
target_include_directories(FazeEngine PRIVATE ${INCLUDE_DIRS})
target_link_libraries(FazeEngine PRIVATE ${LIBRARIES})

# Copy the resource files to the output directory
file(COPY "${CMAKE_SOURCE_DIR}/res" DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
