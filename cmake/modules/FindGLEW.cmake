# Searchs the GLEW library
# This module will set the next variables:
#   GLEW_FOUND          - GLEW was succesfully found
#   GLEW_INCLUDE_DIRS   - GLEW header "GL/glew.h"
#   GLEW_LIBRARIES      - GLEW libraries

# Common directories where the GLEW headers could be
set(glew_INCLUDE_DIRS
    "/usr/include"
    "/usr/local/include"
)

# Search for the headers
find_path(
    glew_INCLUDE_DIR
    "GL/glew.h"
    PATHS "${glew_INCLUDE_DIRS}"
)

# Common directories where the GLEW lib could be
set(glew_LIBRARY_DIRS
    "/usr/lib"
    "/usr/local/lib"
)

# Search for the libs
find_library(
    glew_LIBRARY
    NAMES "GLEW" "glew32" "glew" "glew32s"
    PATHS "${glew_LIBRARY_DIRS}"
)

# We set the GLEW_FOUND and other arguments of find_package with the next lines
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    glew
    REQUIRED_VARS glew_INCLUDE_DIR glew_LIBRARY
)

# If found, set the needed variables
if(GLEW_FOUND)
    set(GLEW_INCLUDE_DIRS "${glew_INCLUDE_DIR}")
    set(GLEW_LIBRARIES "${glew_LIBRARY}")

    # output
    message(STATUS "FindGLEW -> include: ${GLEW_INCLUDE_DIRS} | lib: ${GLEW_LIBRARIES}")
endif(GLEW_FOUND)

