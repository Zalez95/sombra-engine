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

		/** The starting log level */
		static constexpr LogLevel kLogLevel = LogLevel::Trace;

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


#define FORMAT_LOCATION(function, line) function << "(" << line << "): "
#define LOCATION FORMAT_LOCATION(__FUNCTION__, __LINE__)

#define SOMBRA_TRACE_LOG	\
	se::utils::Log::getInstance()(se::utils::LogLevel::Trace)	<< LOCATION
#define SOMBRA_DEBUG_LOG	\
	se::utils::Log::getInstance()(se::utils::LogLevel::Debug)	<< LOCATION
#define SOMBRA_INFO_LOG		\
	se::utils::Log::getInstance()(se::utils::LogLevel::Info)	<< LOCATION
#define SOMBRA_WARN_LOG		\
	se::utils::Log::getInstance()(se::utils::LogLevel::Warning)	<< LOCATION
#define SOMBRA_ERROR_LOG	\
	se::utils::Log::getInstance()(se::utils::LogLevel::Error)	<< LOCATION
#define SOMBRA_FATAL_LOG	\
	se::utils::Log::getInstance()(se::utils::LogLevel::Fatal)	<< LOCATION

#endif		// LOG_H
