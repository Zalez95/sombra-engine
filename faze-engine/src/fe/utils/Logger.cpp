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
		static const std::string timeFormat = "[%Y/%m/%d %H:%M:%S]";

		// Lock the log mutex
		std::lock_guard<std::mutex> locker(mMutex);

		// Check if the text should be written with the current log level
		if (level < mMaxLogLevel) { return; }

		// Get current time
		time_t t = time(0);
		tm* now = localtime(&t);

		// Write the current time
		std::string timeStr;
		timeStr.resize(timeFormat.size());
		strftime(&timeStr[0], timeStr.size(), timeFormat.c_str(), now);
		mLogFile << timeStr;

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
