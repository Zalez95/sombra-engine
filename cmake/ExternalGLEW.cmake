include(ExternalProject)

# Download the project
ExternalProject_Add(glewDownload
	DOWNLOAD_COMMAND	git submodule update --init "${EXTERNAL_PATH}/glew"
	SOURCE_DIR			"${EXTERNAL_PATH}/glew"
	INSTALL_DIR			"${EXTERNAL_INSTALL_PATH}/glew"
	CMAKE_ARGS			-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
						-Dglew-cmake_BUILD_MULTI_CONTEXT=OFF
						-Dglew-cmake_BUILD_SHARED=${BUILD_SHARED_LIBS}
						-Dglew-cmake_BUILD_STATIC=$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>
)

# Get the properties from the downloaded target
ExternalProject_Get_Property(glewDownload INSTALL_DIR)

set(GLEW_FOUND TRUE)
set(GLEW_INCLUDE_DIR "${INSTALL_DIR}/include")
set(glewLibName "glew")
if(${EXTERNAL_BUILD_TYPE} MATCHES "Debug")
	set(glewLibName "glewd")
endif()
if(BUILD_SHARED_LIBS)
	set(GLEW_LIBRARY "${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}${glewLibName}${CMAKE_SHARED_LIBRARY_SUFFIX}")
else()
	set(GLEW_LIBRARY "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${glewLibName}${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

# Create the target and add its properties
add_library(glew INTERFACE)
target_include_directories(glew INTERFACE ${GLEW_INCLUDE_DIR})
target_link_libraries(glew INTERFACE ${GLEW_LIBRARY})
add_dependencies(glew glewDownload)
