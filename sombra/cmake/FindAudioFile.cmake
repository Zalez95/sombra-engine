# Searchs the AUDIOFILE library
# This module will set the next variables:
#	AUDIOFILE_FOUND			- AUDIOFILE was succesfully found
#	AUDIOFILE_INCLUDE_DIR	- AUDIOFILE header "AudioFile.h"
#	AUDIOFILE_LIBRARY		- AUDIOFILE libraries
#	AudioFile				- AUDIOFILE target

include(FindPackageHandleStandardArgs)

# Search for the headers
find_path(
	AUDIOFILE_INCLUDE_DIR
	QUIET
	NAMES "AudioFile.h"
)

# Search for the libs
find_library(
	AUDIOFILE_LIBRARY
	QUIET
	NAMES "audioFile" "AudioFile"
)

find_package_handle_standard_args(
	AudioFile
	REQUIRED_VARS AUDIOFILE_INCLUDE_DIR AUDIOFILE_LIBRARY
)

if(AUDIOFILE_FOUND)
	# Create the dependency target
	add_library(AudioFile UNKNOWN IMPORTED)
	set_target_properties(AudioFile PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES ${AUDIOFILE_INCLUDE_DIR}
		IMPORTED_LOCATION ${AUDIOFILE_LIBRARY}
	)
endif()
