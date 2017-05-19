# Searchs the GLM library
# This module will set the next variables:
#   GLM_FOUND          - GLM was succesfully found
#   GLM_INCLUDE_DIRS   - GLM header "GL/glm.h"

# Common directories where the GLM headers could be
set(glm_INCLUDE_DIRS
    "/usr/include"
    "/usr/local/include"
)

# Search for the headers
find_path(
    glm_INCLUDE_DIR
    "glm/glm.hpp"
    PATHS "${glm_INCLUDE_DIRS}"
)

# We set the GLM_FOUND and other arguments of find_package with the next lines
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    glm
    REQUIRED_VARS glm_INCLUDE_DIR
)

# If found, set the needed variables
if(GLM_FOUND)
    set(GLM_INCLUDE_DIRS "${glm_INCLUDE_DIR}")

    # output
    message(STATUS "FindGLM -> include: ${GLM_INCLUDE_DIRS}")
endif(GLM_FOUND)

