# Searchs the MINIAUDIO library
# This module will set the next variables:
#	MINIAUDIO_FOUND			- MINIAUDIO was succesfully found
#	MINIAUDIO_INCLUDE_DIR	- MINIAUDIO header "miniaudio.h"
#	miniaudio				- MINIAUDIO target

include(FindPackageHandleStandardArgs)

# Search for the headers
find_path(
	MINIAUDIO_INCLUDE_DIR
	QUIET
	NAMES "miniaudio.h"
)

find_package_handle_standard_args(
	MINIAUDIO
	REQUIRED_VARS MINIAUDIO_INCLUDE_DIR
)

if(MINIAUDIO_FOUND)
	# Create the dependency target
	add_library(miniaudio INTERFACE)
	set_target_properties(miniaudio PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES ${MINIAUDIO_INCLUDE_DIR}
	)
endif()
