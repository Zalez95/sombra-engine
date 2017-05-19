# Searchs the FreeImage library
# This module will set the next variables:
#   FreeImage_FOUND          - FreeImage was succesfully found
#   FreeImage_INCLUDE_DIRS   - FreeImage header "GL/FreeImage.h"
#   FreeImage_LIBRARIES      - FreeImage libraries

# Common directories where the FreeImage headers could be
set(FreeImage_INCLUDE_DIRS
    "/usr/include"
    "/usr/local/include"
)

# Search for the headers
find_path(
    FreeImage_INCLUDE_DIR
    "FreeImage.h"
    PATHS "${FreeImage_INCLUDE_DIRS}"
)

# Common directories where the FreeImage lib could be
set(FreeImage_LIBRARY_DIRS
    "/usr/lib"
    "/usr/local/lib"
)

# Search for the libs
find_library(
    FreeImage_LIBRARY
    NAMES "FreeImage" "freeimage"
    PATHS "${FreeImage_LIBRARY_DIRS}"
)

# We set the FreeImage_FOUND and other arguments of find_package with the next lines
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    FreeImage
    REQUIRED_VARS FreeImage_INCLUDE_DIR FreeImage_LIBRARY
)

# If found, set the needed variables
if(FreeImage_FOUND)
    set(FreeImage_INCLUDE_DIRS "${FreeImage_INCLUDE_DIR}")
    set(FreeImage_LIBRARIES "${FreeImage_LIBRARY}")

    # output
    message(STATUS "FindFreeImage -> include: ${FreeImage_INCLUDE_DIRS} | lib: ${FreeImage_LIBRARIES}")
endif(FreeImage_FOUND)

