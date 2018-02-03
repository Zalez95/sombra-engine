include(ExternalProject)

# Download the project
ExternalProject_Add(soilDownload
	DOWNLOAD_COMMAND	git submodule update --init "${EXTERNAL_PATH}/soil"
	SOURCE_DIR			"${EXTERNAL_PATH}/soil"
	INSTALL_DIR			"${EXTERNAL_INSTALL_PATH}/soil"
	CMAKE_ARGS			-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
						-DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
						-Wno-dev
)

# Get the properties from the downloaded target
ExternalProject_Get_Property(soilDownload INSTALL_DIR)

set(SOIL_FOUND TRUE)
set(SOIL_INCLUDE_DIR "${INSTALL_DIR}/include")
if(BUILD_SHARED_LIBS)
	set(SOIL_LIBRARY "${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}SOIL${CMAKE_SHARED_LIBRARY_SUFFIX}")
else()
	set(SOIL_LIBRARY "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}SOIL${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

# Create the target and add its properties
add_library(soil INTERFACE)
target_include_directories(soil INTERFACE ${SOIL_INCLUDE_DIR})
target_link_libraries(soil INTERFACE ${SOIL_LIBRARY})
add_dependencies(soil soilDownload)
