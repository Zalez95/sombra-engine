include(ExternalProject)

# Download the project
ExternalProject_Add(gtestDownload
	DOWNLOAD_COMMAND	git submodule update --init "${EXTERNAL_PATH}/gtest"
	SOURCE_DIR			"${EXTERNAL_PATH}/gtest"
	INSTALL_DIR			"${EXTERNAL_INSTALL_PATH}/gtest"
	CMAKE_ARGS			-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
						-DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
						-DBUILD_GTEST=ON
						-DBUILD_GMOCK=OFF
						-Wno-dev
)

# Get the properties from the downloaded target
ExternalProject_Get_Property(gtestDownload INSTALL_DIR)

set(GTEST_FOUND TRUE)
set(GTEST_INCLUDE_DIR "${INSTALL_DIR}/include")
if(BUILD_SHARED_LIBS)
	set(GTEST_LIBRARY "${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}gtest${CMAKE_SHARED_LIBRARY_SUFFIX}")
else()
	set(GTEST_LIBRARY "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}gtest${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

# Create the target and add its properties
add_library(gtest INTERFACE)
target_include_directories(gtest INTERFACE ${GTEST_INCLUDE_DIR})
target_link_libraries(gtest INTERFACE ${GTEST_LIBRARY})
add_dependencies(gtest gtestDownload)
