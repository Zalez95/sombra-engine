#include "se/utils/Log.h"

namespace se::utils {

	Logger& Log::getInstance()
	{
		static Logger instance = createLogger();
		return instance;
	}

// Private functions
	Logger Log::createLogger()
	{
		Logger logger(kLogFile);
		logger.setLogLevel(kLogLevel);
		return logger;
	}

}
