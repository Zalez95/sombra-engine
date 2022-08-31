include(FetchContent)

FetchContent_Declare(
	GTest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG release-1.12.1
	GIT_SHALLOW TRUE
)
FetchContent_GetProperties(GTest)
if(NOT gtest_POPULATED)
	FetchContent_Populate(GTest)

	set(BUILD_GTEST ON CACHE INTERNAL "")
	set(BUILD_GMOCK OFF CACHE INTERNAL "")
	if(FORCE_STATIC_VCRT)
		set(gtest_force_shared_crt OFF CACHE INTERNAL "")
	else()
		set(gtest_force_shared_crt ON CACHE INTERNAL "")
	endif()
	set(FORCE_STATIC_VCRT ${FORCE_STATIC_VCRT} CACHE INTERNAL "")
	set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE INTERNAL "")
	set(CMAKE_BUILD_TYPE ${CONFIG} CACHE INTERNAL "")
	set(CMAKE_DEBUG_POSTFIX ${MY_DEBUG_POSTFIX} CACHE INTERNAL "")
	set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS} CACHE INTERNAL "")

	add_subdirectory(${gtest_SOURCE_DIR} ${gtest_BINARY_DIR})
endif()
