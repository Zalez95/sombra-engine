include(FetchContent)

FetchContent_Declare(
	NlohmannJSON
	GIT_REPOSITORY https://github.com/nlohmann/json.git
	GIT_TAG v3.5.0
	GIT_SHALLOW TRUE
)
FetchContent_GetProperties(NlohmannJSON)
if(NOT nlohmannjson_POPULATED)
	FetchContent_Populate(NlohmannJSON)

	set(JSON_BuildTests OFF CACHE INTERNAL "")
	set(JSON_MultipleHeaders ON CACHE INTERNAL "")
	set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE INTERNAL "")
	set(CMAKE_BUILD_TYPE ${CONFIG} CACHE INTERNAL "")

	add_subdirectory(${nlohmannjson_SOURCE_DIR} ${nlohmannjson_BINARY_DIR})
endif()
