include(ExternalProject)

# Download the project
ExternalProject_Add(glewDownload
	DOWNLOAD_COMMAND	git submodule update --init "${SOMBRA_EXT_PATH}/glew"
	SOURCE_DIR			"${SOMBRA_EXT_PATH}/glew"
	INSTALL_DIR			"${SOMBRA_EXT_INSTALL_PATH}/glew"
	CMAKE_ARGS			-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
						-DCMAKE_BUILD_TYPE=$<CONFIG>
						-DCMAKE_DEBUG_POSTFIX=${MY_DEBUG_POSTFIX}
						-Dglew-cmake_BUILD_MULTI_CONTEXT=OFF
						-Dglew-cmake_BUILD_SHARED=$<BOOL:${BUILD_SHARED_LIBS}>
						-Dglew-cmake_BUILD_STATIC=$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>
						-DONLY_LIBS=ON
)

# Get the properties from the downloaded target
ExternalProject_Get_Property(glewDownload INSTALL_DIR)

set(GLEW_FOUND TRUE)
set(GLEW_INCLUDE_DIR "${INSTALL_DIR}/include/")
set(GLEW_LIBRARY_DIR "${INSTALL_DIR}/lib/")
if(BUILD_SHARED_LIBS)
	set(GLEW_LIBRARY "${CMAKE_SHARED_LIBRARY_PREFIX}glew${CMAKE_SHARED_LIBRARY_SUFFIX}")
	set(GLEW_DEBUG_LIBRARY "${CMAKE_SHARED_LIBRARY_PREFIX}glew${MY_DEBUG_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}")
else()
	set(GLEW_LIBRARY "${CMAKE_STATIC_LIBRARY_PREFIX}glew${CMAKE_STATIC_LIBRARY_SUFFIX}")
	set(GLEW_DEBUG_LIBRARY "${CMAKE_STATIC_LIBRARY_PREFIX}glew${MY_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

# Create the target and add its properties
add_library(glew INTERFACE)
target_include_directories(glew INTERFACE ${GLEW_INCLUDE_DIR})
target_link_libraries(glew INTERFACE
	optimized "${GLEW_LIBRARY_DIR}${GLEW_LIBRARY}"
	debug "${GLEW_LIBRARY_DIR}${GLEW_DEBUG_LIBRARY}"
)
add_dependencies(glew glewDownload)
