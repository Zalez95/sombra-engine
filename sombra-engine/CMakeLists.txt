include(GNUInstallDirs)

# Find the library source files
file(GLOB_RECURSE SOMBRA_ENGINE_SOURCES "src/*.cpp")

# Create the library
add_library(SombraEngine ${SOMBRA_ENGINE_SOURCES})

# Add the include directories
target_include_directories(SombraEngine
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
	PRIVATE "src"
)

# Add the compiler options
set_target_properties(SombraEngine PROPERTIES
	CXX_STANDARD			17
	CXX_STANDARD_REQUIRED	On
	DEBUG_POSTFIX			${MY_DEBUG_POSTFIX}
)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	target_compile_options(SombraEngine PRIVATE -Wall -Wextra -Wpedantic)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	target_compile_options(SombraEngine PRIVATE /W4 -D_CRT_SECURE_NO_WARNINGS)
endif()

# Add the compiler definitions
target_compile_definitions(SombraEngine PUBLIC GLM_FORCE_SILENT_WARNINGS GLM_ENABLE_EXPERIMENTAL GLEW_NO_GLU)
if(NOT BUILD_SHARED_LIBS)
	target_compile_definitions(SombraEngine PUBLIC AL_LIBTYPE_STATIC GLEW_STATIC)
endif()

# Link the dependencies
target_link_libraries(SombraEngine PUBLIC glm AudioFile NlohmannJSON FreeType PRIVATE OpenAL stb glfw glew OpenGL)

# Install the target
install(TARGETS SombraEngine
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Create the tests
if(SOMBRA_ENGINE_BUILD_TESTS)
	# Find the tests source files
	file(GLOB_RECURSE SOMBRA_ENGINE_TEST_SOURCES "test/*.cpp")

	# Create the executable
	add_executable(SombraEngineTest ${SOMBRA_ENGINE_TEST_SOURCES})

	# Add the include directories
	target_include_directories(SombraEngineTest PRIVATE "test")

	# Add the compiler options
	set_target_properties(SombraEngineTest PROPERTIES
		CXX_STANDARD			17
		CXX_STANDARD_REQUIRED	On
		DEBUG_POSTFIX			${MY_DEBUG_POSTFIX}
	)
	if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
		target_compile_options(SombraEngineTest PRIVATE -Wall -Wextra -Werror)
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
		target_compile_options(SombraEngineTest PRIVATE /W4 -D_CRT_SECURE_NO_WARNINGS)
	endif()

	# Link the dependencies
	target_link_libraries(SombraEngineTest PRIVATE GTest SombraEngine)

	# Add the test
	add_test(SombraEngineTest SombraEngineTest)

	# Install the target
	install(TARGETS SombraEngineTest DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

# Create the documentation
if(SOMBRA_ENGINE_BUILD_DOC AND DOXYGEN_FOUND)
	set(DOXYGEN_IN "${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in")
	set(DOXYGEN_OUT "${CMAKE_CURRENT_BINARY_DIR}/doc")

	configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
	add_custom_target(SombraEngineDoc ALL
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/doc"
		COMMENT "Generating API documentation with Doxygen"
	)
endif()
