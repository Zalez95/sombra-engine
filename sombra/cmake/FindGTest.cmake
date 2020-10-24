# Searchs the Google Test library
# This module will set the next variables:
#	GTEST_FOUND			- GTEST was succesfully found
#	GTEST_INCLUDE_DIR	- GTEST header "gtest/gtest.h"
#	GTEST_LIBRARY		- GTEST libraries
#	GTest				- GTEST target

include(FindPackageHandleStandardArgs)

# Search for the headers
find_path(
	GTEST_INCLUDE_DIR
	QUIET
	NAMES "gtest/gtest.h"
)

# Search for the libs
find_library(
	GTEST_LIBRARY
	QUIET
	NAMES "gtest"
)

find_package_handle_standard_args(
	GTest
	REQUIRED_VARS GTEST_INCLUDE_DIR GTEST_LIBRARY
)

if(GTEST_FOUND)
	# Create the dependency target
	add_library(GTest UNKNOWN IMPORTED)
	set_target_properties(GTest PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES ${GTEST_INCLUDE_DIR}
		IMPORTED_LOCATION ${GTEST_LIBRARY}
	)
endif()
