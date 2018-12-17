include(ExternalProject)

# Download the project
ExternalProject_Add(NlohmannJSONDownload
	DOWNLOAD_COMMAND	git submodule update --init -- "${EXTERNAL_PATH}/nlohmann-json"
	SOURCE_DIR			"${EXTERNAL_PATH}/nlohmann-json"
	INSTALL_DIR			"${EXTERNAL_INSTALL_PATH}/nlohmann-json"
	CMAKE_ARGS			-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
						-DCMAKE_BUILD_TYPE=$<CONFIG>
)

# Get the properties from the downloaded target
ExternalProject_Get_Property(NlohmannJSONDownload INSTALL_DIR)

set(NLOHMANN_JSON_FOUND TRUE)
set(NLOHMANN_JSON_INCLUDE_DIR "${INSTALL_DIR}/include")

# Create the target and add its properties
add_library(NlohmannJSON INTERFACE)
target_include_directories(NlohmannJSON INTERFACE ${NLOHMANN_JSON_INCLUDE_DIR})
add_dependencies(NlohmannJSON NlohmannJSONDownload)
