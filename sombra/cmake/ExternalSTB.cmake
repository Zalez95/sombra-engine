include(FetchContent)

FetchContent_Declare(
	STB
	GIT_REPOSITORY https://github.com/Zalez95/stb.git
	GIT_TAG v1.0-cmake
	GIT_SHALLOW TRUE
)
FetchContent_GetProperties(STB)
if(NOT stb_POPULATED)
	FetchContent_Populate(STB)

	set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE INTERNAL "")
	set(CMAKE_BUILD_TYPE ${CONFIG} CACHE INTERNAL "")

	add_subdirectory(${stb_SOURCE_DIR} ${stb_BINARY_DIR})
endif()
