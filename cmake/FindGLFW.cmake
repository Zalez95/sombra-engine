# Searchs the GLFW library
# This module will set the next variables:
#   GLFW_FOUND			- GLFW was succesfully found
#   GLFW_INCLUDE_DIR	- GLFW3 header "GLFW/glfw3.h"
#   GLFW_LIBRARY		- GLFW libraries

include(FindPackageHandleStandardArgs)

# Search for the headers
find_path(
	GLFW_INCLUDE_DIR
	QUIET
	NAMES "GLFW/glfw3.h"
)

# Search for the libs
find_library(
	GLFW_LIBRARY
	QUIET
	NAMES "glfw" "glfw3" "glfw32" "glfw32s"
)

find_package_handle_standard_args(
	glfw
	REQUIRED_VARS	GLFW_INCLUDE_DIR GLFW_LIBRARY
)
