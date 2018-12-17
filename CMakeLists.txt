cmake_minimum_required(VERSION 3.8.2 FATAL_ERROR)

###############################################################################
# 								SOMBRA ENGINE
###############################################################################
# Project name and version
project(SombraEngine VERSION 0.5.0 LANGUAGES CXX)

# Project options
option(SOMBRA_ENGINE_BUILD_DOC "Generate the SombraEngine documentation" ON)
option(SOMBRA_ENGINE_BUILD_TESTS "Build the SombraEngine test programs" ON)

# Set the project module directories
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Set the external libraries configuration data
set(EXTERNAL_PATH "${CMAKE_SOURCE_DIR}/ext")
set(EXTERNAL_INSTALL_PATH "${CMAKE_BINARY_DIR}/ext")

# Global compiler options
set(MY_DEBUG_POSTFIX "d")
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	option(FORCE_STATIC_VCRT "Force /MT for static VC runtimes" OFF)
	if(NOT BUILD_SHARED_LIBS AND FORCE_STATIC_VCRT)
		foreach(flagVar
			CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
			CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
		)
			if(${flagVar} MATCHES "/MD")
				string(REGEX REPLACE "/MD" "/MT" ${flagVar} "${${flagVar}}")
			endif()
		endforeach()
	endif()
endif()

# Include the dependencies
include(Dependencies)

# Create the libraries and the executables
enable_testing()
add_subdirectory(sombra-engine)
add_subdirectory(game)
