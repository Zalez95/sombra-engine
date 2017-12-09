# Searchs the SOIL library
# This module will set the next variables:
#   SOIL_FOUND          - SOIL was succesfully found
#   SOIL_INCLUDE_DIRS   - SOIL header "SOIL/SOIL.h"
#   SOIL_LIBRARIES      - SOIL libraries

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
	soil
	REQUIRED_VARS	SOIL_INCLUDE_DIR SOIL_LIBRARY
)
