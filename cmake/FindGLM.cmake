# Searchs the GLM library
# This module will set the next variables:
#	GLM_FOUND		- GLM was succesfully found
#	GLM_INCLUDE_DIR	- GLM header "glm/glm.hpp"

# Search for the headers
find_path(
	GLM_INCLUDE_DIR
	QUIET
	NAMES "glm/glm.hpp"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
	glm
	REQUIRED_VARS GLM_INCLUDE_DIR
)

