# Searchs the SOIL library
# This module will set the next variables:
#   SOIL_FOUND          - SOIL was succesfully found
#   SOIL_INCLUDE_DIRS   - SOIL header "GL/SOIL.h"
#   SOIL_LIBRARIES      - SOIL libraries

# Common directories where the SOIL headers could be
set(SOIL_INCLUDE_DIRS
    "/usr/include"
    "/usr/local/include"
)

# Search for the headers
find_path(
    SOIL_INCLUDE_DIR
    "SOIL.h"
    PATHS "${SOIL_INCLUDE_DIRS}"
)

# Common directories where the SOIL lib could be
set(SOIL_LIBRARY_DIRS
    "/usr/lib"
    "/usr/local/lib"
)

# Search for the libs
find_library(
    SOIL_LIBRARY
    NAMES "SOIL" "soil"
    PATHS "${SOIL_LIBRARY_DIRS}"
)

# We set the SOIL_FOUND and other arguments of find_package with the next lines
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    SOIL
    REQUIRED_VARS SOIL_INCLUDE_DIR SOIL_LIBRARY
)

# If found, set the needed variables
if(SOIL_FOUND)
    set(SOIL_INCLUDE_DIRS "${SOIL_INCLUDE_DIR}")
    set(SOIL_LIBRARIES "${SOIL_LIBRARY}")

    # output
    message(STATUS "FindSOIL -> include: ${SOIL_INCLUDE_DIRS} | lib: ${SOIL_LIBRARIES}")
endif(SOIL_FOUND)

