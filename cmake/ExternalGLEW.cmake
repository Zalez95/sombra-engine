include(ExternalProject)
ExternalProject_Add(
	glew
	GIT_REPOSITORY	"https://github.com/Perlmint/glew-cmake"
	GIT_TAG			"glew-cmake-1.11.0"
	SOURCE_DIR		"${EXTERNAL_PATH}/glew"
	INSTALL_DIR		"${EXTERNAL_INSTALL_LOCATION}"
	UPDATE_COMMAND	""
	PATCH_COMMAND	""
	TEST_COMMAND	""
	CMAKE_ARGS		"-DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}"
					"-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
)

set(GLEW_FOUND TRUE)
set(GLEW_INCLUDE_DIR "${EXTERNAL_INSTALL_LOCATION}/include")
if (BUILD_SHARED_LIBS)
	set(GLEW_LIBRARY "${EXTERNAL_INSTALL_LOCATION}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}glew${CMAKE_SHARED_LIBRARY_SUFFIX}")
else()
	set(GLEW_LIBRARY "${EXTERNAL_INSTALL_LOCATION}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}glew${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()
