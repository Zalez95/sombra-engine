#include <string>
#include <AL/alc.h>
#include "se/utils/Log.h"

#define ALC_WRAP(x, device)				\
	se::audio::alcClearError(device);	\
	x;									\
	se::audio::alcLogError(device, #x, LOCATION);


namespace se::audio {

	static void alcClearError(ALCdevice* device)
	{
		while (alcGetError(device) != ALC_NO_ERROR);
	}


	static bool alcLogError(ALCdevice* device, const char* alcFunction, const std::string& location)
	{
		ALCenum error = alcGetError(device);
		while (error != ALC_NO_ERROR) {
			utils::Log::getInstance()(utils::LogLevel::Error) << location
				<< "Audio Library Context function \"" << alcFunction << "\" returned error code " << error
				<< ": \"" << alGetString(error) << "\"";
			return true;
		}

		return false;
	}

}
