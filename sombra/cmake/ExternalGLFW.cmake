include(ExternalProject)

# Download the project
ExternalProject_Add(glfwDownload
	DOWNLOAD_COMMAND	git submodule update --init "${SOMBRA_EXT_PATH}/glfw"
	SOURCE_DIR			"${SOMBRA_EXT_PATH}/glfw"
	INSTALL_DIR			"${SOMBRA_EXT_INSTALL_PATH}/glfw"
	CMAKE_ARGS			-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
						-DCMAKE_BUILD_TYPE=$<CONFIG>
						-DCMAKE_DEBUG_POSTFIX=${MY_DEBUG_POSTFIX}
						-DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
						-DGLFW_BUILD_DOCS=OFF
						-DGLFW_BUILD_TESTS=OFF
						-DGLFW_BUILD_EXAMPLES=OFF
						-DUSE_MSVC_RUNTIME_LIBRARY_DLL=$<NOT:$<BOOL:${FORCE_STATIC_VCRT}>>
						--no-warn-unused-cli
)

# Get the properties from the downloaded target
ExternalProject_Get_Property(glfwDownload INSTALL_DIR)

set(GLFW_FOUND TRUE)
set(GLFW_INCLUDE_DIR "${INSTALL_DIR}/include/")
set(GLFW_LIBRARY_DIR "${INSTALL_DIR}/lib/")
if(BUILD_SHARED_LIBS)
	set(GLFW_LIBRARY "${CMAKE_SHARED_LIBRARY_PREFIX}glfw3${CMAKE_SHARED_LIBRARY_SUFFIX}")
	set(GLFW_DEBUG_LIBRARY "${CMAKE_SHARED_LIBRARY_PREFIX}glfw3${MY_DEBUG_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}")
else()
	set(GLFW_LIBRARY "${CMAKE_STATIC_LIBRARY_PREFIX}glfw3${CMAKE_STATIC_LIBRARY_SUFFIX}")
	set(GLFW_DEBUG_LIBRARY "${CMAKE_STATIC_LIBRARY_PREFIX}glfw3${MY_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

# Add the system dependencies of the library
if(UNIX)
	find_package(Threads REQUIRED)
	find_package(X11 REQUIRED)

	if (NOT X11_Xrandr_FOUND)
		message(FATAL_ERROR "The RandR library wasn't found")
	endif()
	if (NOT X11_Xinerama_FOUND)
		message(FATAL_ERROR "The Xinerama library wasn't found")
	endif()
	if (NOT X11_xf86vmode_FOUND)
		message(FATAL_ERROR "The xf86vmode library wasn't found")
	endif()
	if (NOT X11_Xcursor_FOUND)
		message(FATAL_ERROR "The Xcursor library wasn't found")
	endif()

	list(APPEND SYSTEM_LIBRARIES
		${X11_X11_LIB} ${X11_Xrandr_LIB} ${X11_Xinerama_LIB} ${X11_Xxf86vm_LIB} ${X11_Xcursor_LIB}
		${CMAKE_THREAD_LIBS_INIT}
		${CMAKE_DL_LIBS}
	)

	list(APPEND GLFW_LIBRARY ${SYSTEM_LIBRARIES})
	list(APPEND GLFW_DEBUG_LIBRARY ${SYSTEM_LIBRARIES})
endif()

# Create the target and add its properties
add_library(glfw INTERFACE)
target_include_directories(glfw INTERFACE ${GLFW_INCLUDE_DIR})
target_link_libraries(glfw INTERFACE
	optimized "${GLFW_LIBRARY_DIR}${GLFW_LIBRARY}"
	debug "${GLFW_LIBRARY_DIR}${GLFW_DEBUG_LIBRARY}"
)
add_dependencies(glfw glfwDownload)
