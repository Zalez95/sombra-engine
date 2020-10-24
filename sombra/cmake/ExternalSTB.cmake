include(ExternalProject)

# Download the project
ExternalProject_Add(stbDownload
	DOWNLOAD_COMMAND	git submodule update --init -- "${SOMBRA_EXT_PATH}/stb"
	SOURCE_DIR			"${SOMBRA_EXT_PATH}/stb"
	INSTALL_DIR			"${SOMBRA_EXT_INSTALL_PATH}/stb"
	CMAKE_ARGS			-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
						-DCMAKE_BUILD_TYPE=$<CONFIG>
)

# Get the properties from the downloaded target
ExternalProject_Get_Property(stbDownload INSTALL_DIR)

set(STB_FOUND TRUE)
set(STB_INCLUDE_DIR "${INSTALL_DIR}/include")

# Create the target and add its properties
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${STB_INCLUDE_DIR})
add_dependencies(stb stbDownload)
