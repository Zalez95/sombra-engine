# Searchs the GLEW library
# This module will set the next variables:
#   GLEW_FOUND			- GLEW was succesfully found
#   GLEW_INCLUDE_DIR	- GLEW header "GL/glew.h"
#   GLEW_LIBRARY		- GLEW libraries

include(FindPackageHandleStandardArgs)

# Search for the headers
find_path(
	GLEW_INCLUDE_DIR
	QUIET
	NAMES "GL/glew.h"
)

# Search for the libs
find_library(
	GLEW_LIBRARY
	QUIET
	NAMES "GLEW" "glew32"
)

find_package_handle_standard_args(
	glew
	REQUIRED_VARS	GLEW_INCLUDE_DIR GLEW_LIBRARY
)

