#include <ctime>
#include <chrono>
#include <iomanip>
#include "fe/utils/Logger.h"

namespace fe { namespace utils {

	Logger::~Logger()
	{
		mLogFile.close();
	}


	Logger& Logger::getInstance()
	{
		static Logger mInstance(LOG_PATH);		// Create the static instance
		return mInstance;
	}


	void Logger::write(LogLevel level, const std::string& text)
	{
		// Lock for writing in the log file and formatting the time
		// (tm is a static variable, so it isn't thread safe)
		std::lock_guard<std::mutex> locker(mMutex);

		// Check if the text should be written with the current log level
		if (level < mMaxLogLevel) { return; }

		// Write current time
		time_t t = time(0);
		tm* now = localtime(&t);
		mLogFile.fill('0');
		mLogFile << '[' << now->tm_year + 1900
			<< '/' << std::setw(2) << now->tm_mon + 1
			<< '/' << std::setw(2) << now->tm_mday
			<< ' ' << std::setw(2) << now->tm_hour
			<< ':' << std::setw(2) << now->tm_min
			<< ':' << std::setw(2) << now->tm_sec
			<< ']';

		// Write the level of the log text
		switch (level)
		{
		case LogLevel::WARNING:
			mLogFile << " [WARNING]";
			break;
		case LogLevel::ERROR:
			mLogFile << " [ERROR]";
			break;
		case LogLevel::DEBUG:
			mLogFile << " [DEBUG]";
			break;
		}

		// Write the text
		mLogFile << '\t' << text << std::endl;
	}

}}
