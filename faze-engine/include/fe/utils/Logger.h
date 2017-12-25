#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>

#define LOG_PATH "./log.txt"

namespace fe { namespace utils {

	/**
	 * The type of the text that we can write in the Log File with the Logger
	 */
	enum LogType
	{
		WARNING,
		ERROR,
		DEBUG
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

	public:		// Functions
		/** Class destructor */
		~Logger();

		/** Writes the given text with given label and the current time to the
		 * Log File
		 *
		 * @note	the Logger write automatically the end of line
		 * @param	type the label of the Log text
		 * @param	text the text that we want to write to the log file */
		static void writeLog(LogType type, const std::string& text);
	private:
		/** Class constructor, it's private for preventing construction */
		Logger(const std::string& logPath);

		/** Constructor-Copy object, it's private for preventing construction by
		 * copy */
		Logger(const Logger&);

		/** Writes the given text in the log file with also the current time
		 *
		 * @param	type the type of the text that we are going to write
		 * @param	text the text that we want to write in the log file */
		void write(LogType type, const std::string& text);
	};

}}

#endif		// LOGGER_H
