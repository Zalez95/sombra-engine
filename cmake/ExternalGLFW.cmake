include(ExternalProject)

# Download the project
ExternalProject_Add(glfwDownload
	DOWNLOAD_COMMAND	git submodule update --init "${EXTERNAL_PATH}/glfw"
	SOURCE_DIR			"${EXTERNAL_PATH}/glfw"
	INSTALL_DIR			"${EXTERNAL_INSTALL_PATH}/glfw"
	CMAKE_ARGS			-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
						-DGLFW_BUILD_DOCS=OFF
						-DGLFW_BUILD_TESTS=OFF
						-DGLFW_BUILD_EXAMPLES=OFF
						-DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
)

# Get the properties from the downloaded target
ExternalProject_Get_Property(glfwDownload INSTALL_DIR)

set(GLFW_FOUND TRUE)
set(GLFW_INCLUDE_DIR "${INSTALL_DIR}/include")
if(BUILD_SHARED_LIBS)
	set(GLFW_LIBRARY "${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}glfw3${CMAKE_SHARED_LIBRARY_SUFFIX}")
else()
	set(GLFW_LIBRARY "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}glfw3${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

# Add the system dependencies of the library
if(UNIX)
	find_package(Threads REQUIRED)
	find_package(
		X11 REQUIRED
		COMPONENTS X11_xf86vmode X11_Xrandr X11_Xinerama X11_Xinput X11_Xcursor
	)

	list(APPEND GLFW_LIBRARY
		${X11_LIBRARIES} ${X11_Xxf86vm_LIB} ${X11_Xinerama_LIB} ${X11_Xrandr_LIB} ${X11_Xinput_LIB} ${X11_Xcursor_LIB}
		${CMAKE_THREAD_LIBS_INIT}
		${CMAKE_DL_LIBS}
	)
endif()

# Create the target and add its properties
add_library(glfw INTERFACE)
target_include_directories(glfw INTERFACE ${GLFW_INCLUDE_DIR})
target_link_libraries(glfw INTERFACE ${GLFW_LIBRARY})
add_dependencies(glfw glfwDownload)
