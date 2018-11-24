#include <string>
#include <AL/al.h>
#include "se/utils/Logger.h"

#define AL_WRAP(x)					\
	se::audio::alClearError();		\
	x;								\
	se::audio::alLogError(#x);


namespace se::audio {

	static void alClearError()
	{
		while (alGetError() != AL_NO_ERROR);
	}


	static bool alLogError(const std::string& functionName)
	{
		ALenum error = alGetError();
		while (error != AL_NO_ERROR) {
			utils::Logger::getInstance().write(
				utils::LogLevel::ERROR,
				"OpenAL function \"" + functionName + "\" returned error: "
					+ std::to_string(error)
			);
			return true;
		}

		return false;
	}

}
