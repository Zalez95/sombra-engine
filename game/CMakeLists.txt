# Find the executable source files
set(GAME_SOURCES "src/game/Game.cpp" "src/main.cpp")

# Create the executable
add_executable(game ${GAME_SOURCES})

# Add the include directories
target_include_directories(game PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
	$<INSTALL_INTERFACE:include>
)

# Add the compiler options
target_compile_options(game PRIVATE -std=c++14)
if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
	target_compile_options(game PRIVATE -Wall -Wextra -Wpedantic -Werror)
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "MSVC")
	target_compile_options(game PRIVATE /W4 /MT -D_CRT_SECURE_NO_WARNINGS)
	set_target_properties(game PROPERTIES CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
	set_target_properties(game PROPERTIES CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()
set_target_properties(game PROPERTIES DEBUG_POSTFIX d)

# Link the dependencies
target_link_libraries(game PRIVATE fazeEngine)

# Copy the resource files to the output directory
file(COPY "res" DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
