include(FetchContent)

FetchContent_Declare(
	Miniaudio
	GIT_REPOSITORY https://github.com/Zalez95/miniaudio.git
	GIT_TAG 0.11.8-cmake
	GIT_SHALLOW TRUE
)
FetchContent_GetProperties(Miniaudio)
if(NOT miniaudio_POPULATED)
	FetchContent_Populate(Miniaudio)

	set(MINIAUDIO_BUILD_TESTS OFF CACHE INTERNAL "")
	set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE INTERNAL "")
	set(CMAKE_BUILD_TYPE ${CONFIG} CACHE INTERNAL "")

	add_subdirectory(${miniaudio_SOURCE_DIR} ${miniaudio_BINARY_DIR})
endif()
