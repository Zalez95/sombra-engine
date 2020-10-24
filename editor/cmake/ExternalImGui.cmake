include(ExternalProject)

# Download the project
ExternalProject_Add(imguiDownload
	DOWNLOAD_COMMAND	git submodule update --init "${EDITOR_EXT_PATH}/imgui"
	SOURCE_DIR			"${EDITOR_EXT_PATH}/imgui"
	INSTALL_DIR			"${EDITOR_EXT_INSTALL_PATH}/imgui"
	CMAKE_ARGS			-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
						-DCMAKE_BUILD_TYPE=$<CONFIG>
						-DCMAKE_DEBUG_POSTFIX=${MY_DEBUG_POSTFIX}
						-DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
						-DIMGUI_BUILD_EXAMPLES=OFF
						-DUSE_MSVC_RUNTIME_LIBRARY_DLL=$<NOT:$<BOOL:${FORCE_STATIC_VCRT}>>
						--no-warn-unused-cli
)

# Get the properties from the downloaded target
ExternalProject_Get_Property(imguiDownload INSTALL_DIR)

set(IMGUI_FOUND TRUE)
set(IMGUI_INCLUDE_DIR "${INSTALL_DIR}/include/")
set(IMGUI_LIBRARY_DIR "${INSTALL_DIR}/lib/")
if(BUILD_SHARED_LIBS)
	set(IMGUI_LIBRARY "${CMAKE_SHARED_LIBRARY_PREFIX}imgui${CMAKE_SHARED_LIBRARY_SUFFIX}")
	set(IMGUI_DEBUG_LIBRARY "${CMAKE_SHARED_LIBRARY_PREFIX}imgui${MY_DEBUG_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}")
else()
	set(IMGUI_LIBRARY "${CMAKE_STATIC_LIBRARY_PREFIX}imgui${CMAKE_STATIC_LIBRARY_SUFFIX}")
	set(IMGUI_DEBUG_LIBRARY "${CMAKE_STATIC_LIBRARY_PREFIX}imgui${MY_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

# Create the target and add its properties
add_library(imgui INTERFACE)
target_include_directories(imgui INTERFACE ${IMGUI_INCLUDE_DIR})
target_link_libraries(imgui INTERFACE
	optimized "${IMGUI_LIBRARY_DIR}${IMGUI_LIBRARY}"
	debug "${IMGUI_LIBRARY_DIR}${IMGUI_DEBUG_LIBRARY}"
)
add_dependencies(imgui imguiDownload)
