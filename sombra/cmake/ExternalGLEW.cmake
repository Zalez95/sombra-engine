include(FetchContent)

FetchContent_Declare(
	GLEW
	GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
	GIT_TAG glew-cmake-2.2.0
	GIT_SHALLOW TRUE
)
FetchContent_GetProperties(GLEW)
if(NOT glew_POPULATED)
	FetchContent_Populate(GLEW)

	set(glew-cmake_BUILD_MULTI_CONTEXT OFF CACHE INTERNAL "")
	set(glew-cmake_BUILD_SHARED ${BUILD_SHARED_LIBS} CACHE INTERNAL "")
	if(BUILD_SHARED_LIBS)
		set(glew-cmake_BUILD_STATIC OFF CACHE INTERNAL "")
	else()
		set(glew-cmake_BUILD_STATIC ON CACHE INTERNAL "")
	endif()
	set(ONLY_LIBS ON CACHE INTERNAL "")
	set(FORCE_STATIC_VCRT ${FORCE_STATIC_VCRT} CACHE INTERNAL "")
	set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE INTERNAL "")
	set(CMAKE_BUILD_TYPE ${CONFIG} CACHE INTERNAL "")
	set(CMAKE_DEBUG_POSTFIX ${MY_DEBUG_POSTFIX} CACHE INTERNAL "")

	add_subdirectory(${glew_SOURCE_DIR} ${glew_BINARY_DIR})

	add_library(glew INTERFACE)
	if(BUILD_SHARED_LIBS)
		add_dependencies(glew libglew_shared)
		target_link_libraries(glew INTERFACE libglew_shared)
	else()
		add_dependencies(glew libglew_static)
		target_link_libraries(glew INTERFACE libglew_static)
	endif()
endif()
