# Searchs the NLOHMANN_JSON library
# This module will set the next variables:
#	NLOHMANN_JSON_FOUND			- NLOHMANN_JSON was succesfully found
#	NLOHMANN_JSON_INCLUDE_DIR	- NLOHMANN_JSON header "nlohmann/json.hpp"
#	NlohmannJSON				- NLOHMANN_JSON target

include(FindPackageHandleStandardArgs)

# Search for the headers
find_path(
	NLOHMANN_JSON_INCLUDE_DIR
	QUIET
	NAMES "nlohmann/json.hpp"
)

find_package_handle_standard_args(
	NlohmannJSON
	REQUIRED_VARS NLOHMANN_JSON_INCLUDE_DIR
)

if(NLOHMANN_JSON_FOUND)
	# Create the dependency target
	add_library(NlohmannJSON INTERFACE)
	set_target_properties(NlohmannJSON PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES ${NLOHMANN_JSON_INCLUDE_DIR}
	)
endif()
