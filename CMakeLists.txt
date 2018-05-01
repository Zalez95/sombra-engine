cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

###############################################################################
# 								FAZE ENGINE
###############################################################################
# Project name and version
project(FazeEngine VERSION 0.5.0 LANGUAGES CXX)

# Project options
option(FAZE_ENGINE_BUILD_DOC "Generate the FazeEngine documentation" ON)
option(FAZE_ENGINE_BUILD_TESTS "Build the Faze Engine test programs" ON)

# Set the project module directories
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Set the project output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
foreach(outputConfig ${CMAKE_CONFIGURATION_TYPES})
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${outputConfig} "${CMAKE_SOURCE_DIR}/bin")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${outputConfig} "${CMAKE_SOURCE_DIR}/lib")
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${outputConfig} "${CMAKE_SOURCE_DIR}/lib")
endforeach()

# Set the external libraries configuration data
option(BUILD_SHARED_LIBS
	"Use the SHARED version of the third party libraries if possible" OFF)
set(EXTERNAL_PATH "${CMAKE_SOURCE_DIR}/ext")
set(EXTERNAL_INSTALL_PATH "${CMAKE_BINARY_DIR}/ext")

# Other constants
set(MY_DEBUG_POSTFIX "d")

# Include the dependencies
include(Dependencies)

# Create the libraries and the executables
add_subdirectory(faze-engine)
add_subdirectory(game)
