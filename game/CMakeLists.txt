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
set_target_properties(game PROPERTIES
	CXX_STANDARD			17
	CXX_STANDARD_REQUIRED	On
	DEBUG_POSTFIX			${MY_DEBUG_POSTFIX}
)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	target_compile_options(game PRIVATE -Wall -Wextra -Wpedantic -Werror)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	target_compile_options(game PRIVATE /W4 /Za -D_CRT_SECURE_NO_WARNINGS)
endif()

# Link the dependencies
target_link_libraries(game PRIVATE sombraEngine)
