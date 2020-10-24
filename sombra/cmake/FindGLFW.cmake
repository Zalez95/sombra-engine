# Searchs the GLFW library
# This module will set the next variables:
#	GLFW_FOUND			- GLFW was succesfully found
#	GLFW_INCLUDE_DIR	- GLFW3 header "GLFW/glfw3.h"
#	GLFW_LIBRARY		- GLFW libraries
#	glfw				- GLFW target

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
	REQUIRED_VARS GLFW_INCLUDE_DIR GLFW_LIBRARY
)

if(GLFW_FOUND)
	# Create the dependency target
	add_library(glfw UNKNOWN IMPORTED)
	set_target_properties(glfw PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES ${GLFW_INCLUDE_DIR}
		IMPORTED_LOCATION ${GLFW_LIBRARY}
	)
endif()
