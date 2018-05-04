# Find the library source files
file(GLOB_RECURSE FAZE_ENGINE_SOURCES "src/*.cpp")

# Create the library
add_library(fazeEngine ${FAZE_ENGINE_SOURCES})

# Add the include directories
target_include_directories(fazeEngine PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
	PRIVATE "src"
)

# Add the compiler options
set_target_properties(fazeEngine PROPERTIES
	CXX_STANDARD			14
	CXX_STANDARD_REQUIRED	On
	DEBUG_POSTFIX			${MY_DEBUG_POSTFIX}
)
if(CMAKE_CXX_COMPILER MATCHES "GNU")
	target_compile_options(fazeEngine PRIVATE -Wall -Wextra -Wpedantic -Werror)
elseif(CMAKE_CXX_COMPILER MATCHES "MSVC")
	target_compile_options(fazeEngine PRIVATE /W4 /Za -D_CRT_SECURE_NO_WARNINGS)
endif()

# Add the compiler definitions
target_compile_definitions(fazeEngine PRIVATE GLEW_NO_GLU)
if(NOT BUILD_SHARED_LIBS)
	target_compile_definitions(fazeEngine PRIVATE AL_LIBTYPE_STATIC GLEW_STATIC)
endif()

# Link the dependencies
target_include_directories(fazeEngine PUBLIC glm PRIVATE audioFile openal soil glfw glew)
target_link_libraries(fazeEngine PUBLIC glm audioFile openal soil glfw glew opengl)

# Create the tests
if(FAZE_ENGINE_BUILD_TESTS)
	# Find the tests source files
	file(GLOB_RECURSE FAZE_ENGINE_TEST_SOURCES "test/*.cpp")

	# Create the executable
	add_executable(fazeEngineTest ${FAZE_ENGINE_TEST_SOURCES})

	# Add the compiler options
	set_target_properties(fazeEngineTest PROPERTIES
		CXX_STANDARD			14
		CXX_STANDARD_REQUIRED	On
		DEBUG_POSTFIX			${MY_DEBUG_POSTFIX}
	)
	if(CMAKE_CXX_COMPILER MATCHES "GNU")
		target_compile_options(fazeEngineTest PRIVATE -Wall -Wextra -Werror)
	elseif(CMAKE_CXX_COMPILER MATCHES "MSVC")
		target_compile_options(fazeEngineTest PRIVATE /W4 /Za -D_CRT_SECURE_NO_WARNINGS)
	endif()

	# Link the dependencies
	target_link_libraries(fazeEngineTest PRIVATE gtest fazeEngine)

	# Add the test
	add_test(fazeEngineTest fazeEngineTest)
endif()

# Create the documentation
if(FAZE_ENGINE_BUILD_DOC AND DOXYGEN_FOUND)
	set(DOXYGEN_IN "${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in")
	set(DOXYGEN_OUT "${CMAKE_CURRENT_BINARY_DIR}/doc")

	configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
	add_custom_target(fazeEngineDoc ALL
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/doc"
		COMMENT "Generating API documentation with Doxygen"
	)
endif()
