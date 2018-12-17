# Searchs the SOIL library
# This module will set the next variables:
#	SOIL_FOUND			- SOIL was succesfully found
#	SOIL_INCLUDE_DIR	- SOIL header "SOIL/SOIL.h"
#	SOIL_LIBRARY		- SOIL libraries
#	Soil				- SOIL target

include(FindPackageHandleStandardArgs)

# Search for the headers
find_path(
	SOIL_INCLUDE_DIR
	QUIET
	NAMES "SOIL/SOIL.h"
)

# Search for the libs
find_library(
	SOIL_LIBRARY
	QUIET
	NAMES "SOIL" "soil"
)

find_package_handle_standard_args(
	Soil
	REQUIRED_VARS SOIL_INCLUDE_DIR SOIL_LIBRARY
)

if(SOIL_FOUND)
	# Create the dependency target
	add_library(Soil UNKNOWN IMPORTED)
	set_target_properties(Soil PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES ${SOIL_INCLUDE_DIR}
		IMPORTED_LOCATION ${SOIL_LIBRARY}
	)
endif()
