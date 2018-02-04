#ifndef LOGGER_H
#define LOGGER_H

#include <mutex>
#include <string>
#include <fstream>

#define LOG_PATH "./log.txt"

namespace fe { namespace utils {

	/**
	 * The type of the text that we can write in the Log File with the Logger
	 */
	enum class LogLevel : unsigned int
	{
		DEBUG = 0,
		WARNING,
		ERROR
	};


	/**
	 * Logger class, it's used for register log data in the log file that
	 * is located in the current directory. It follows the Singleton pattern,
	 * so it can only exists one instance of this class at the same time.
	 */
	class Logger
	{
	private:	// Attributes
		/** The only posible instance of the Logger class */
		static Logger mInstance;

		/** The mutex of the log file */
		std::mutex mMutex;

		/** The log file */
		std::ofstream mLogFile;

		/** The maximum dept of the text logs to write in mLogFile */
		LogLevel mMaxLogLevel;

	public:		// Functions
		/** Class destructor */
		~Logger();

		/** @return	a reference to the only posible instance of the Logger.
		 * @note	it creates the instance if it doesn't exist yet */
		static Logger& getInstance();

		/** Changes the maximum log level to show in the log file
		 * @param	level the new maximum level */
		inline void setLogLevel(LogLevel level) { mMaxLogLevel = level; };

		/** Writes the given text in the log file with also the current time
		 *
		 * @param	level the level of the log text that we are going to write
		 * @param	text the text that we want to write in the log file
		 * @note	if the level of a message is lower than the maximum level
		 *			it won't be written */
		void write(LogLevel level, const std::string& text);
	private:
		/** Class constructor, it's private for preventing construction */
		Logger(const std::string& logPath) :
			mLogFile(logPath, std::ios::app),
			mMaxLogLevel(LogLevel::DEBUG) {}

		/** Constructor-Copy object, it's private for preventing construction by
		 * copy */
		Logger(const Logger&);
	};

}}

#endif		// LOGGER_H
