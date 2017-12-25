include(ExternalProject)

# Download the project
ExternalProject_Add(
	gtestDownload
	GIT_REPOSITORY	"https://github.com/google/googletest.git"
	GIT_TAG			"release-1.8.0"
	SOURCE_DIR		"${EXTERNAL_PATH}/gtest"
	CMAKE_ARGS		"-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
					"-DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}"
	INSTALL_DIR		"${EXTERNAL_INSTALL_LOCATION}"
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
