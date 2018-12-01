#include "se/utils/Log.h"

namespace se::utils {

	Logger& Log::getInstance()
	{
		static Logger instance( std::move(createLogger()) );
		return instance;
	}

// Private functions
	Logger Log::createLogger()
	{
		Logger logger(kLogFile);
		logger.setLogLevel(LogLevel::Debug);
		return logger;
	}

}
