include(GNUInstallDirs)

# Include the dependencies
set(EDITOR_EXT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ext")
set(EDITOR_EXT_INSTALL_PATH "${CMAKE_BINARY_DIR}/editor/ext")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Dependencies)

# Find the executable source files
file(GLOB_RECURSE EDITOR_SOURCES "src/*.cpp")

# Create the executable
add_executable(Editor ${EDITOR_SOURCES})

# Add the include directories
target_include_directories(Editor PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
	$<INSTALL_INTERFACE:include>
)

# Add the compiler options
set_target_properties(Editor PROPERTIES
	CXX_STANDARD			17
	CXX_STANDARD_REQUIRED	On
	DEBUG_POSTFIX			${MY_DEBUG_POSTFIX}
)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	target_compile_options(Editor PRIVATE -Wall -Wextra -Wpedantic)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	target_compile_options(Editor PRIVATE /W4 -D_CRT_SECURE_NO_WARNINGS)
endif()

# Link the dependencies
target_link_libraries(Editor PRIVATE Sombra imgui)

# Install the target
install(TARGETS Editor DESTINATION ${CMAKE_INSTALL_BINDIR})
