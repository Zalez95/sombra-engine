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
target_compile_options(fazeEngine PRIVATE -std=c++14)
if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
	target_compile_options(fazeEngine PRIVATE -Wall -Wextra -Wpedantic -Werror)
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "MSVC")
	target_compile_options(fazeEngine PRIVATE /W4 /MT -D_CRT_SECURE_NO_WARNINGS)
	set_target_properties(fazeEngine PROPERTIES CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
	set_target_properties(fazeEngine PROPERTIES CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
endif()
set_target_properties(fazeEngine PROPERTIES DEBUG_POSTFIX d)

# Add the compiler definitions
set_target_properties(fazeEngine PROPERTIES COMPILE_DEFINITIONS GLEW_STATIC)

# Link the dependencies
target_link_libraries(fazeEngine PUBLIC glm soil glfw glew opengl)

# Create the tests
if (FAZE_ENGINE_BUILD_TESTS)
	# Find the tests source files
	file(GLOB_RECURSE FAZE_ENGINE_TEST_SOURCES "test/*.cpp")

	# Create the executable
	add_executable(fazeEngineTest ${FAZE_ENGINE_TEST_SOURCES})
	
	# Add the compiler options
	target_compile_options(fazeEngineTest PRIVATE -std=c++14)
	target_compile_options(fazeEngineTest PRIVATE -std=c++14)
	if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
		target_compile_options(fazeEngineTest PRIVATE -Wall -Wextra -Wpedantic -Werror)
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
