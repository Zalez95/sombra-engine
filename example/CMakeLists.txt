include(GNUInstallDirs)

# Find the executable source files
file(GLOB_RECURSE EXAMPLE_SOURCES "src/*.cpp")

# Create the executable
add_executable(Example ${EXAMPLE_SOURCES})

# Add the include directories
target_include_directories(Example PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
	$<INSTALL_INTERFACE:include>
)

# Add the compiler options
set_target_properties(Example PROPERTIES
	CXX_STANDARD			17
	CXX_STANDARD_REQUIRED	On
	DEBUG_POSTFIX			${MY_DEBUG_POSTFIX}
)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	target_compile_options(Example PRIVATE "-Wall" "-Wextra" "-Wpedantic")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	target_compile_options(Example PRIVATE "/W4" "-D_CRT_SECURE_NO_WARNINGS")
endif()

# Link the dependencies
target_link_libraries(Example PRIVATE Sombra)

# Install the target
install(TARGETS Example DESTINATION ${CMAKE_INSTALL_BINDIR})
