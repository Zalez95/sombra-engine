#include <string>
#include <AL/al.h>
#include "se/utils/Log.h"

#define AL_WRAP(x)					\
	se::audio::alClearError();		\
	x;								\
	se::audio::alLogError(#x, LOCATION);


namespace se::audio {

	static void alClearError()
	{
		while (alGetError() != AL_NO_ERROR);
	}


	static bool alLogError(const char* alFunction, const std::string& location)
	{
		ALenum error = alGetError();
		while (error != AL_NO_ERROR) {
			utils::Log::getInstance()(utils::LogLevel::Error) << location
				<< "OpenAL function \"" << alFunction << "\" returned error code " << error
				<< ": \"" << alGetString(error) << "\"";
			return true;
		}

		return false;
	}

}
