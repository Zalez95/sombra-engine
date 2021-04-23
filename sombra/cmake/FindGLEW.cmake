# Searchs the GLEW library
# This module will set the next variables:
#	GLEW_FOUND			- GLEW was succesfully found
#	GLEW_INCLUDE_DIR	- GLEW header "GL/glew.h"
#	GLEW_LIBRARY		- GLEW libraries
#	glew				- GLEW target

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
	GLEW
	REQUIRED_VARS GLEW_INCLUDE_DIR GLEW_LIBRARY
)

if(GLEW_FOUND)
	# Create the dependency target
	add_library(glew UNKNOWN IMPORTED)
	set_target_properties(glew PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES ${GLEW_INCLUDE_DIR}
		IMPORTED_LOCATION ${GLEW_LIBRARY}
	)
endif()
