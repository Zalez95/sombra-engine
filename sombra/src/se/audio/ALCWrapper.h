#include <string>
#include <AL/alc.h>
#include "se/utils/Log.h"

#ifdef NDEBUG
	#define ALC_WRAP(x) x
#else
	#define ALC_WRAP(x, device)				\
		se::audio::alcClearError(device);	\
		x;									\
		se::audio::alcLogError(device, #x, __FUNCTION__, __LINE__)
#endif

namespace se::audio {

	inline void alcClearError(ALCdevice* device)
	{
		while (alcGetError(device) != ALC_NO_ERROR);
	}


	inline void alcLogError(ALCdevice* device, const char* alcFunction, const char* function, int line)
	{
		ALCenum error;
		while ((error = alcGetError(device)) != ALC_NO_ERROR) {
			utils::Log::getInstance()(utils::LogLevel::Error) << FORMAT_LOCATION(function, line)
				<< "Audio Library Context function \"" << alcFunction << "\" returned error code " << error
				<< ": \"" << alGetString(error) << "\"";
		}
	}

}
