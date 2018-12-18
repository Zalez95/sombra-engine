#include <ctime>
#include <chrono>
#include <thread>
#include <iomanip>
#include "se/utils/Logger.h"

namespace se::utils {

	Logger::Logger(Logger&& other) :
		mLogFile(std::move(other.mLogFile)), mMaxLogLevel(other.mMaxLogLevel) {}


	Logger& Logger::operator=(Logger&& other)
	{
		mLogFile = std::move(other.mLogFile);
		mMaxLogLevel = other.mMaxLogLevel;
		return *this;
	}


	LogStream Logger::operator()(LogLevel level)
	{
		return LogStream(*this, level);
	}


	void Logger::write(LogLevel level, const std::string& text)
	{
		// Check if the text should be written with the current log level
		if (level < mMaxLogLevel) { return; }

		// Get the level label
		std::string label;
		switch (level) {
			case LogLevel::Trace:	label = "TRACE";	break;
			case LogLevel::Debug:	label = "DEBUG";	break;
			case LogLevel::Info:	label = "INFO";		break;
			case LogLevel::Warning:	label = "WARNING";	break;
			case LogLevel::Error:	label = "ERROR";	break;
		}

		// Write to the log file
		std::lock_guard<std::mutex> locker(mMutex);
		mLogFile
			<< getTimeString()
			<< " [" << label << "]"
			<< std::hex << " 0x" << std::this_thread::get_id()
			<< "\t" << text << "\n";
	}

// Private functions
	std::string Logger::getTimeString()
	{
		static const char timeFormat[] = "%Y/%m/%d %H:%M:%S";
		using namespace std::chrono;

		system_clock::time_point now = system_clock::now();
		system_clock::duration tp = now.time_since_epoch();
		tp -= duration_cast<seconds>(tp);

		std::ostringstream ss;
		std::time_t tt = system_clock::to_time_t(now);
		ss	<< std::put_time(std::localtime(&tt), timeFormat)
			<< '.' << std::setw(3) << std::setfill('0') << tp / milliseconds(1);

		return ss.str();
	}

}
