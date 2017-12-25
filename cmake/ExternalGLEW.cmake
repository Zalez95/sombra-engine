include(ExternalProject)

set(glewShared OFF)
set(glewStatic OFF)
if(${BUILD_SHARED_LIBS})
	set(glewShared ON)
else()
	set(glewStatic ON)
endif()

set(glewName "glew")
if(${EXTERNAL_BUILD_TYPE} MATCHES "Debug")
	set(glewName "glewd")
endif()

# Download the project
ExternalProject_Add(
	glewDownload
	GIT_REPOSITORY	"https://github.com/Perlmint/glew-cmake.git"
	GIT_TAG			"eda0eed"
	SOURCE_DIR		"${EXTERNAL_PATH}/glew"
	CMAKE_ARGS		"-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
					"-Dglew-cmake_BUILD_SHARED:BOOL=${glewShared}"
					"-Dglew-cmake_BUILD_STATIC:BOOL=${glewStatic}"
)

# Get the properties from the downloaded target
ExternalProject_Get_Property(glewDownload INSTALL_DIR)

set(GLEW_FOUND TRUE)
set(GLEW_INCLUDE_DIR "${INSTALL_DIR}/include")
if(BUILD_SHARED_LIBS)
	set(GLEW_LIBRARY "${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}${glewName}${CMAKE_SHARED_LIBRARY_SUFFIX}")
else()
	set(GLEW_LIBRARY "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${glewName}${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

# Create the target and add its properties
add_library(glew INTERFACE)
target_include_directories(glew INTERFACE ${GLEW_INCLUDE_DIR})
target_link_libraries(glew INTERFACE ${GLEW_LIBRARY})
add_dependencies(glew glewDownload)
