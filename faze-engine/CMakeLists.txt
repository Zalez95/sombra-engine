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
set_target_properties(fazeEngine PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED On)
if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
	target_compile_options(fazeEngine PRIVATE -Wall -Wextra -Wpedantic -Werror)
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "MSVC")
	target_compile_options(fazeEngine PRIVATE /W4 /MT -D_CRT_SECURE_NO_WARNINGS)
	set_target_properties(fazeEngine PROPERTIES CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
	set_target_properties(fazeEngine PROPERTIES CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
endif()
set_target_properties(fazeEngine PROPERTIES DEBUG_POSTFIX d)

# Add the compiler definitions
target_compile_definitions(fazeEngine PUBLIC GLEW_STATIC GLEW_NO_GLU)

# Link the dependencies
target_include_directories(fazeEngine PUBLIC glm PRIVATE soil glfw glew)
target_link_libraries(fazeEngine PUBLIC glm soil glfw glew opengl)

# Create the tests
if(FAZE_ENGINE_BUILD_TESTS)
	# Find the tests source files
	file(GLOB_RECURSE FAZE_ENGINE_TEST_SOURCES "test/*.cpp")

	# Create the executable
	add_executable(fazeEngineTest ${FAZE_ENGINE_TEST_SOURCES})

	# Add the compiler options
	set_target_properties(fazeEngineTest PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED On)
	if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
		target_compile_options(fazeEngineTest PRIVATE -Wall -Wextra -Werror)
	elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "MSVC")
		target_compile_options(fazeEngineTest PRIVATE /W4 /MT -D_CRT_SECURE_NO_WARNINGS)
		set_target_properties(fazeEngineTest PROPERTIES CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
		set_target_properties(fazeEngineTest PROPERTIES CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
	endif()
	set_target_properties(fazeEngineTest PROPERTIES DEBUG_POSTFIX d)

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
