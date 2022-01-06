#include <string>
#include <AL/al.h>
#include "se/utils/Log.h"

#ifdef NDEBUG
	#define AL_WRAP(x) x
#else
	#define AL_WRAP(x)					\
		se::audio::alClearError();		\
		x;								\
		se::audio::alLogError(#x, __FUNCTION__, __LINE__)
#endif

namespace se::audio {

	inline void alClearError()
	{
		while (alGetError() != AL_NO_ERROR);
	}


	inline void alLogError(const char* alFunction, const char* function, int line)
	{
		ALenum error;
		while ((error = alGetError()) != AL_NO_ERROR) {
			std::string errorTag = "";
			switch (error) {
				case AL_NO_ERROR:			errorTag = "AL_NO_ERROR";			break;
				case AL_INVALID_NAME:		errorTag = "AL_INVALID_NAME";		break;
				case AL_INVALID_ENUM:		errorTag = "AL_INVALID_ENUM";		break;
				case AL_INVALID_VALUE:		errorTag = "AL_INVALID_VALUE";		break;
				case AL_INVALID_OPERATION:	errorTag = "AL_INVALID_OPERATION";	break;
				case AL_OUT_OF_MEMORY:		errorTag = "AL_OUT_OF_MEMORY";		break;
			}

			utils::Log::getInstance()(utils::LogLevel::Error) << FORMAT_LOCATION(function, line)
				<< "OpenAL function \"" << alFunction << "\" returned error code " << error
				<< " (" << errorTag << "): \"" << alGetString(error) << "\"";
		}
	}

}
