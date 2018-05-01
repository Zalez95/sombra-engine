include(ExternalProject)

# Download the project
ExternalProject_Add(audioFileDownload
	DOWNLOAD_COMMAND	git submodule update --init "${EXTERNAL_PATH}/audiofile"
	SOURCE_DIR			"${EXTERNAL_PATH}/audiofile"
	INSTALL_DIR			"${EXTERNAL_INSTALL_PATH}/audiofile"
	CMAKE_ARGS			-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
						-DCMAKE_BUILD_TYPE=$<CONFIG>
						-DCMAKE_DEBUG_POSTFIX=${MY_DEBUG_POSTFIX}
						-DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
						-DAUDIOFILE_BUILD_TESTS=OFF
)

# Get the properties from the downloaded target
ExternalProject_Get_Property(audioFileDownload INSTALL_DIR)

set(AUDIOFILE_FOUND TRUE)
set(AUDIOFILE_INCLUDE_DIR "${INSTALL_DIR}/include")
set(AUDIOFILE_LIBRARY_DIR "${INSTALL_DIR}/lib/")
if(BUILD_SHARED_LIBS)
	set(AUDIOFILE_LIBRARY "${CMAKE_SHARED_LIBRARY_PREFIX}audioFile${CMAKE_SHARED_LIBRARY_SUFFIX}")
	set(AUDIOFILE_DEBUG_LIBRARY "${CMAKE_SHARED_LIBRARY_PREFIX}audioFile${MY_DEBUG_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}")
else()
	set(AUDIOFILE_LIBRARY "${CMAKE_STATIC_LIBRARY_PREFIX}audioFile${CMAKE_STATIC_LIBRARY_SUFFIX}")
	set(AUDIOFILE_DEBUG_LIBRARY "${CMAKE_STATIC_LIBRARY_PREFIX}audioFile${MY_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

# Create the target and add its properties
add_library(audioFile INTERFACE)
target_include_directories(audioFile INTERFACE ${AUDIOFILE_INCLUDE_DIR})
target_link_libraries(audioFile INTERFACE
	optimized "${AUDIOFILE_LIBRARY_DIR}${AUDIOFILE_LIBRARY}"
	debug "${AUDIOFILE_LIBRARY_DIR}${AUDIOFILE_DEBUG_LIBRARY}"
)
add_dependencies(audioFile audioFileDownload)
