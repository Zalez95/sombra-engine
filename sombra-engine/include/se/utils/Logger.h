#ifndef LOGGER_H
#define LOGGER_H

#include <mutex>
#include <ostream>
#include <fstream>
#include "StringUtils.h"

namespace se::utils {

	template <typename T, std::streamsize S>
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
	private:	// Nested types
		using MyStream = LogStream<char, 1024>;

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
		Logger(const char* path) :
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
		MyStream operator()(LogLevel level);

		/** Writes the given stream to the log file with other metadata like the
		 * date, time, log level and thread id
		 *
		 * @param	stream with the text to write to the log file
		 * @note	if the level of a stream is lower than the maximum level
		 *			it won't be written */
		void write(const MyStream& stream);

		/** Changes the maximum log level to show in the log file
		 * @param	level the new maximum level */
		void setLogLevel(LogLevel level) { mMaxLogLevel = level; };
	private:
		/** Appends the current formated time to the given stream
		 *
		 * @param	os the stream where the time will be appended */
		static void putTime(std::ostream& os);
	};


	/**
	 * Class LogStream, it's used to write text to the Logger in a stream like
	 * fashion
	 */
	template <typename CharT, std::streamsize Size>
	class LogStream : public std::basic_ostream<CharT>
	{
	private:	// Attributes
		/** The streambuf used for storing the text */
		ArrayStreambuf<CharT, Size> mASBuf;

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
			std::basic_ostream<CharT>(&mASBuf),
			mLogger(logger), mLevel(level) {};

		/** Class destructor */
		~LogStream() { mLogger.write(*this); };

		/** @return	the LogLevel of the LogStream */
		LogLevel getLevel() const { return mLevel; };

		/** @return	a pointer to the internal buffer of the LogStream */
		const char* c_str() const { return mASBuf.data(); };
	};

}

#endif		// LOGGER_H
