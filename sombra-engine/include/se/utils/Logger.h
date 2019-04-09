#ifndef LOGGER_H
#define LOGGER_H

#include <mutex>
#include <string>
#include <sstream>
#include <fstream>

namespace se::utils {

	class LogStream;


	/**
	 * The log severity levels that we can write to the Log File with the Logger
	 */
	enum class LogLevel : int
	{
		Trace = 0,
		Debug,
		Info,
		Warning,
		Error,
		Fatal
	};


	/**
	 * Logger class, it's used for register log data in the log file that
	 * is located in the current directory. It follows the Singleton pattern,
	 * so it can only exists one instance of this class at the same time.
	 */
	class Logger
	{
	private:	// Attributes
		/** The log file */
		std::ofstream mLogFile;

		/** The mutex for writting to the log file */
		std::mutex mMutex;

		/** The maximum dept of the text logs to write to the mLogFile */
		LogLevel mMaxLogLevel;

	public:		// Functions
		/** Creates a new Logger
		 *
		 * @param	path the path to the log file to write to */
		Logger(const std::string& path) :
			mLogFile(path, std::ios::app), mMaxLogLevel(LogLevel::Debug) {};
		Logger(const Logger& other) = delete;
		Logger(Logger&& other);

		/** Class destructor */
		~Logger() = default;

		/** Assignment operators*/
		Logger& operator=(const Logger& other) = delete;
		Logger& operator=(Logger&& other);

		/** Returns a LogStream used to write to the Logger
		 *
		 * @param	level the LogLevel of the text to write
		 * @return	the new LogStream object */
		LogStream operator()(LogLevel level);

		/** Changes the maximum log level to show in the log file
		 * @param	level the new maximum level */
		inline void setLogLevel(LogLevel level) { mMaxLogLevel = level; };

		/** Writes the given text in the log file with other metadata like the
		 * date, time, log level and thread id
		 *
		 * @param	level the level of the log text that we are going to write
		 * @param	text the text to write to the log file
		 * @note	if the level of a message is lower than the maximum level
		 *			it won't be written */
		void write(LogLevel level, const std::string& text);
	private:
		/** @return	the current time formated as a string */
		static std::string getTimeString();
	};


	/**
	 * Class LogStream, it's used to write text to the Logger in a stream like
	 * fashion
	 */
	class LogStream : public std::ostringstream
	{
	private:	// Attributes
		/** The logger that we will use to write to the log file */
		Logger& mLogger;

		/** The LogLevel of the text to write */
		LogLevel mLevel;

	public:		// Functions
		/** Creates a new LogStream
		 *
		 * @param	logger the Logger to use to write to the log file
		 * @param	level the LogLevel of the text to write */
		LogStream(Logger& logger, LogLevel level) :
			mLogger(logger), mLevel(level) {};

		/** Class destructor */
		~LogStream() { mLogger.write(mLevel, str()); };
	};

}

#endif		// LOGGER_H
