include(FetchContent)

FetchContent_Declare(
	SOL2
	GIT_REPOSITORY https://github.com/ThePhD/sol2.git
	GIT_TAG v3.2.3
	GIT_SHALLOW TRUE
)
FetchContent_GetProperties(SOL2)
if(NOT sol2_POPULATED)
	FetchContent_Populate(SOL2)

	set(SOL2_BUILD_LUA OFF CACHE INTERNAL "")
	if(FORCE_STATIC_VCRT)
		set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE INTERNAL "")
	else()
		set(USE_MSVC_RUNTIME_LIBRARY_DLL ON CACHE INTERNAL "")
	endif()
	set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE INTERNAL "")
	set(CMAKE_BUILD_TYPE ${CONFIG} CACHE INTERNAL "")
	set(CMAKE_DEBUG_POSTFIX ${MY_DEBUG_POSTFIX} CACHE INTERNAL "")
	set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS} CACHE INTERNAL "")

	add_subdirectory(${sol2_SOURCE_DIR} ${sol2_BINARY_DIR})
endif()
