# Searchs the FREETYPE library
# This module will set the next variables:
#	FREETYPE_FOUND			- FREETYPE was succesfully found
#	FREETYPE_INCLUDE_DIR	- FREETYPE header "ft2build.h"
#	FREETYPE_LIBRARY		- FREETYPE libraries
#	FreeType				- FREETYPE target

include(FindPackageHandleStandardArgs)

# Search for the headers
find_path(
	FREETYPE_INCLUDE_DIR
	QUIET
	NAMES "freetype2" "ft2build.h"
)

# Search for the libs
find_library(
	FREETYPE_LIBRARY
	QUIET
	NAMES "freetype"
)

find_package_handle_standard_args(
	FreeType
	REQUIRED_VARS FREETYPE_INCLUDE_DIR FREETYPE_LIBRARY
)

if(FREETYPE_FOUND)
	# Create the dependency target
	add_library(FreeType UNKNOWN IMPORTED)
	set_target_properties(FreeType PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${FREETYPE_INCLUDE_DIR}/freetype2"
		IMPORTED_LOCATION ${FREETYPE_LIBRARY}
	)
endif()
