# Searchs the GLFW3 library
# This module will set the next variables:
#   GLFW3_FOUND          - GLFW3 was succesfully found
#   GLFW3_INCLUDE_DIRS   - GLFW3 header "GLFW/glfw3.h"
#   GLFW3_LIBRARIES      - GLFW3 libraries

# Common directories where the GLFW3 headers could be
set(glfw3_INCLUDE_DIRS
    "/usr/include"
    "/usr/local/include"
)

# Search for the headers
find_path(
    glfw3_INCLUDE_DIR
    "GLFW/glfw3.h"
    PATHS "${glfw3_INCLUDE_DIRS}"
)

# Common directories where the GLFW3 lib could be
set(glfw3_LIBRARY_DIRS
    "/usr/lib"
    "/usr/local/lib"
)

# Search for the libs
find_library(
    glfw3_LIBRARY
    NAMES "glfw" "glfw3" "glfw32" "glfw32s"
    PATHS "${glfw3_LIBRARY_DIRS}"
)

# We set the GLFW3_FOUND and other arguments of find_package with the next lines
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    glfw3
    REQUIRED_VARS glfw3_INCLUDE_DIR glfw3_LIBRARY
)

# If found, set the needed variables
if(GLFW3_FOUND)
    set(GLFW3_INCLUDE_DIRS "${glfw3_INCLUDE_DIR}")
    set(GLFW3_LIBRARIES "${glfw3_LIBRARY}")

    # output
    message(STATUS "FindGLFW3 -> include: ${GLFW3_INCLUDE_DIRS} | lib: ${GLFW3_LIBRARIES}")
endif(GLFW3_FOUND)

