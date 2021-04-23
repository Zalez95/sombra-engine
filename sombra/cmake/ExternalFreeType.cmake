include(FetchContent)

FetchContent_Declare(
	FreeType
	GIT_REPOSITORY https://git.sv.nongnu.org/r/freetype/freetype2.git
	GIT_TAG VER-2-10-1
)
FetchContent_GetProperties(FreeType)
if(NOT freetype_POPULATED)
	FetchContent_Populate(FreeType)

	set(FORCE_STATIC_VCRT ${FORCE_STATIC_VCRT} CACHE INTERNAL "")
	set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE INTERNAL "")
	set(CMAKE_BUILD_TYPE ${CONFIG} CACHE INTERNAL "")
	set(CMAKE_DEBUG_POSTFIX ${MY_DEBUG_POSTFIX} CACHE INTERNAL "")
	set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS} CACHE INTERNAL "")

	add_subdirectory(${freetype_SOURCE_DIR} ${freetype_BINARY_DIR})
endif()
