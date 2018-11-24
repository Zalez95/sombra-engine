#include <string>
#include <AL/alc.h>
#include "se/utils/Logger.h"

#define ALC_WRAP(x, device)				\
	se::audio::alcClearError(device);	\
	x;									\
	se::audio::alcLogError(device, #x);


namespace se::audio {

	static void alcClearError(ALCdevice* device)
	{
		while (alcGetError(device) != ALC_NO_ERROR);
	}


	static bool alcLogError(ALCdevice* device, const std::string& functionName)
	{
		ALCenum error = alcGetError(device);
		while (error != ALC_NO_ERROR) {
			utils::Logger::getInstance().write(
				utils::LogLevel::ERROR,
				"Audio Library Context function \"" + functionName + "\" returned error: "
					+ std::to_string(error)
			);
			return true;
		}

		return false;
	}

}
