include(FetchContent)

FetchContent_Declare(
	GLM
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	GIT_TAG 0.9.9.8
	GIT_SHALLOW TRUE
)
FetchContent_GetProperties(GLM)
if(NOT glm_POPULATED)
	FetchContent_Populate(GLM)

	set(GLM_TEST_ENABLE OFF CACHE INTERNAL "")
	set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE INTERNAL "")
	set(CMAKE_BUILD_TYPE ${CONFIG} CACHE INTERNAL "")

	add_subdirectory(${glm_SOURCE_DIR} ${glm_BINARY_DIR})
endif()
