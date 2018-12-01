#ifndef LOG_H
#define LOG_H

#include "Logger.h"

namespace se::utils {

	/**
	 * Class Log, it's used to create and access our custom Logger
	 */
	class Log
	{
	private:
		/** The path to the log file */
		static constexpr char kLogFile[] = "sombra.log";

	public:
		/** @return	the only instance of the Logger */
		static Logger& getInstance();
	private:
		/** Creates a Logger with our custom configuration
		 *
		 * @return	the new logger */
		static Logger createLogger();
	};

}


#define LOCATION std::string(__FILE__) + ":" + std::to_string(__LINE__) \
	+ " in \"" + __FUNCTION__ + "\": "

#define SOMBRA_DEBUG_LOG	\
	se::utils::Log::getInstance()(se::utils::LogLevel::Debug)	<< LOCATION
#define SOMBRA_INFO_LOG		\
	se::utils::Log::getInstance()(se::utils::LogLevel::Info)	<< LOCATION
#define SOMBRA_WARN_LOG		\
	se::utils::Log::getInstance()(se::utils::LogLevel::Warning)	<< LOCATION
#define SOMBRA_ERROR_LOG	\
	se::utils::Log::getInstance()(se::utils::LogLevel::Error)	<< LOCATION

#endif		// LOG_H
