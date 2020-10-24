# Searchs the GLM library
# This module will set the next variables:
#	GLM_FOUND		- GLM was succesfully found
#	GLM_INCLUDE_DIR	- GLM header "glm/glm.hpp"
#	glm				- GLM target

include(FindPackageHandleStandardArgs)

# Search for the headers
find_path(
	GLM_INCLUDE_DIR
	QUIET
	NAMES "glm/glm.hpp"
)

find_package_handle_standard_args(
	glm
	REQUIRED_VARS GLM_INCLUDE_DIR
)

if(GLM_FOUND)
	# Create the dependency target
	add_library(glm INTERFACE)
	set_target_properties(glm PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES ${GLM_INCLUDE_DIR}
	)
endif()
