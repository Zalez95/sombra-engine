# Searchs the OPENAL library
# This module will set the next variables:
#	OPENAL_FOUND		- OPENAL was succesfully found
#	OPENAL_INCLUDE_DIRS	- OPENAL header "AL/al.h"
#	OPENAL_LIBRARIES	- OPENAL libraries
#	openal				- OPENAL target

include(FindPackageHandleStandardArgs)

# Search for the headers
find_path(
	OPENAL_INCLUDE_DIR
	QUIET
	NAMES "AL/al.h"
)

# Search for the libs
find_library(
	OPENAL_LIBRARY
	QUIET
	NAMES "OpenAL" "al" "openal" "OpenAL32"
)

find_package_handle_standard_args(
	openal
	REQUIRED_VARS OPENAL_INCLUDE_DIR OPENAL_LIBRARY
)

if(OPENAL_FOUND)
	# Create the dependency target
	add_library(openal UNKNOWN IMPORTED)
	set_target_properties(openal PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES ${OPENAL_INCLUDE_DIR}
		IMPORTED_LOCATION ${OPENAL_LIBRARY}
	)
endif()
