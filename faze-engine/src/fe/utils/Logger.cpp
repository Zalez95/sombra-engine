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
		static const char timeFormat[] = "[%Y/%m/%d %H:%M:%S]";

		// Lock the log mutex
		std::lock_guard<std::mutex> locker(mMutex);

		// Check if the text should be written with the current log level
		if (level < mMaxLogLevel) { return; }

		// Write the current time
		std::time_t t = std::time(nullptr);
		std::tm* now = std::localtime(&t);
		mLogFile << std::put_time(now, timeFormat);

		// Write the level of the log text
		switch (level) {
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
