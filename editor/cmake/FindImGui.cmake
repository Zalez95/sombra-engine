# Searchs the GLFW library
# This module will set the next variables:
#	IMGUI_FOUND			- IMGUI was succesfully found
#	IMGUI_INCLUDE_DIR	- IMGUI header "imgui.h"
#	IMGUI_LIBRARY		- IMGUI libraries
#	ImGui				- IMGUI target

include(FindPackageHandleStandardArgs)

# Search for the headers
find_path(
	IMGUI_INCLUDE_DIR
	QUIET
	NAMES "imgui.h"
)

# Search for the libs
find_library(
	IMGUI_LIBRARY
	QUIET
	NAMES "imgui"
)

find_package_handle_standard_args(
	ImGui
	REQUIRED_VARS IMGUI_INCLUDE_DIR IMGUI_LIBRARY
)

if(IMGUI_FOUND)
	# Create the dependency target
	add_library(ImGui UNKNOWN IMPORTED)
	set_target_properties(ImGui PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES ${IMGUI_INCLUDE_DIR}
		IMPORTED_LOCATION ${IMGUI_LIBRARY}
	)
endif()
