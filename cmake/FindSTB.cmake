# Searchs the STB library
# This module will set the next variables:
#	STB_FOUND		- STB was succesfully found
#	STB_INCLUDE_DIR	- STB header "stb_image.h"
#	stb				- STB target

include(FindPackageHandleStandardArgs)

# Search for the headers
find_path(
	STB_INCLUDE_DIR
	QUIET
	NAMES "stb_image.h"
)

find_package_handle_standard_args(
	stb
	REQUIRED_VARS STB_INCLUDE_DIR
)

if(STB_FOUND)
	# Create the dependency target
	add_library(stb INTERFACE)
	set_target_properties(stb PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES ${STB_INCLUDE_DIR}
	)
endif()
