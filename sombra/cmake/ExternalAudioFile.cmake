include(FetchContent)

FetchContent_Declare(
	AudioFile
	GIT_REPOSITORY https://github.com/Zalez95/audiofile.git
	GIT_TAG v1.0.2-cmake
	GIT_SHALLOW TRUE
)
FetchContent_GetProperties(AudioFile)
if(NOT audiofile_POPULATED)
	FetchContent_Populate(AudioFile)

	set(AUDIOFILE_BUILD_TESTS OFF CACHE INTERNAL "")
	if(FORCE_STATIC_VCRT)
		set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE INTERNAL "")
	else()
		set(USE_MSVC_RUNTIME_LIBRARY_DLL ON CACHE INTERNAL "")
	endif()
	set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE INTERNAL "")
	set(CMAKE_BUILD_TYPE ${CONFIG} CACHE INTERNAL "")
	set(CMAKE_DEBUG_POSTFIX ${MY_DEBUG_POSTFIX} CACHE INTERNAL "")
	set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS} CACHE INTERNAL "")

	add_subdirectory(${audiofile_SOURCE_DIR} ${audiofile_BINARY_DIR})
endif()
