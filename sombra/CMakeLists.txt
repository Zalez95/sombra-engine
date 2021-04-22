include(GNUInstallDirs)

# Sombra Options
option(SOMBRA_BUILD_DOC "Generate the Sombra documentation" ON)
option(SOMBRA_BUILD_TESTS "Build the Sombra test programs" ON)

# Include the dependencies
set(SOMBRA_EXT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ext")
set(SOMBRA_EXT_INSTALL_PATH "${CMAKE_BINARY_DIR}/sombra/ext")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Dependencies)

# Find the library source files
file(GLOB_RECURSE SOMBRA_SOURCES "src/*.cpp")

# Create the library
add_library(Sombra ${SOMBRA_SOURCES})

# Add the include directories
target_include_directories(Sombra
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
	PRIVATE "src"
)

# Add the compiler options
set_target_properties(Sombra PROPERTIES
	CXX_STANDARD			17
	CXX_STANDARD_REQUIRED	On
	DEBUG_POSTFIX			${MY_DEBUG_POSTFIX}
)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	target_compile_options(Sombra PRIVATE -Wall -Wextra -Wpedantic)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	target_compile_options(Sombra PRIVATE /W4 -D_CRT_SECURE_NO_WARNINGS)
endif()

# Add the compiler definitions
target_compile_definitions(Sombra PUBLIC GLM_FORCE_SILENT_WARNINGS GLM_ENABLE_EXPERIMENTAL GLEW_NO_GLU)
if(NOT BUILD_SHARED_LIBS)
	target_compile_definitions(Sombra PUBLIC AL_LIBTYPE_STATIC GLEW_STATIC)
endif()

# Link the dependencies
target_link_libraries(Sombra PUBLIC glm AudioFile NlohmannJSON FreeType PRIVATE OpenAL stb glfw glew OpenGL)

# Install the target
install(TARGETS Sombra EXPORT SombraTargets
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Create the tests
if(SOMBRA_BUILD_TESTS)
	# Find the tests source files
	file(GLOB_RECURSE SOMBRA_TEST_SOURCES "test/*.cpp")

	# Create the executable
	add_executable(SombraTest ${SOMBRA_TEST_SOURCES})

	# Add the include directories
	target_include_directories(SombraTest PRIVATE "test")

	# Add the compiler options
	set_target_properties(SombraTest PROPERTIES
		CXX_STANDARD			17
		CXX_STANDARD_REQUIRED	On
		DEBUG_POSTFIX			${MY_DEBUG_POSTFIX}
	)
	if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
		target_compile_options(SombraTest PRIVATE -Wall -Wextra -Werror)
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
		target_compile_options(SombraTest PRIVATE /W4 -D_CRT_SECURE_NO_WARNINGS)
	endif()

	# Link the dependencies
	target_link_libraries(SombraTest PRIVATE GTest Sombra)

	# Add the test
	add_test(SombraTest SombraTest)

	# Install the target
	install(TARGETS SombraTest DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

# Create the documentation
if(SOMBRA_BUILD_DOC AND DOXYGEN_FOUND)
	set(DOXYGEN_IN "${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in")
	set(DOXYGEN_OUT "${CMAKE_CURRENT_BINARY_DIR}/doc")

	configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
	add_custom_target(SombraDoc ALL
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/doc"
		COMMENT "Generating API documentation with Doxygen"
	)
endif()
