cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

###############################################################################
# 								SOMBRA ENGINE
###############################################################################
# Project name and version
project(SombraEngine LANGUAGES CXX)
set(SOMBRA_ENGINE_VERSION_MAJOR "1")
set(SOMBRA_ENGINE_VERSION_MINOR "1")
set(SOMBRA_ENGINE_VERSION "${SOMBRA_ENGINE_VERSION_MAJOR}.${SOMBRA_ENGINE_VERSION_MINOR}")

# Project options
option(SOMBRA_ENGINE_BUILD_EDITOR "Build the SombraEngine editor program" ON)
option(SOMBRA_ENGINE_BUILD_EXAMPLE "Build the SombraEngine example program" ON)

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

# Create the libraries and the executables
enable_testing()
add_subdirectory(sombra)
if(SOMBRA_ENGINE_BUILD_EDITOR)
	add_subdirectory(editor)
endif()
if(SOMBRA_ENGINE_BUILD_EXAMPLE)
	add_subdirectory(example)
endif()
