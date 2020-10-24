include(ExternalProject)

# Download the project
ExternalProject_Add(glmDownload
	DOWNLOAD_COMMAND	git submodule update --init -- "${SOMBRA_EXT_PATH}/glm"
	SOURCE_DIR			"${SOMBRA_EXT_PATH}/glm"
	INSTALL_DIR			"${SOMBRA_EXT_INSTALL_PATH}/glm"
	CMAKE_ARGS			-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
						-DCMAKE_BUILD_TYPE=$<CONFIG>
						-DGLM_TEST_ENABLE=OFF
)

# Get the properties from the downloaded target
ExternalProject_Get_Property(glmDownload INSTALL_DIR)

set(GLM_FOUND TRUE)
set(GLM_INCLUDE_DIR "${INSTALL_DIR}/include")

# Create the target and add its properties
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE ${GLM_INCLUDE_DIR})
add_dependencies(glm glmDownload)
