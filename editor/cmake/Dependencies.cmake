# Sets all the dependencies of the project
option(INSTALLED_IMGUI		"Use installed ImGui library"				ON)

find_package(ImGui QUIET)
if(IMGUI_FOUND AND INSTALLED_IMGUI)
	message("IMGUI FOUND: Using system library")
else()
	message("IMGUI NOT FOUND: Downloading and building ImGui from source")
	include(ExternalImGui)
endif()

message("Downloading and building ImGuizmo from source")
include(ExternalImGuizmo)
