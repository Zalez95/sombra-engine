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


	Logger::MyStream Logger::operator()(LogLevel level)
	{
		return MyStream(*this, level);
	}


	void Logger::write(const MyStream& stream)
	{
		// Check if the text should be written with the current log level
		if (stream.getLevel() < mMaxLogLevel) { return; }

		// Get the level label
		const char* label = "";
		switch (stream.getLevel()) {
			case LogLevel::Trace:	label = "TRACE";	break;
			case LogLevel::Debug:	label = "DEBUG";	break;
			case LogLevel::Info:	label = "INFO ";	break;
			case LogLevel::Warning:	label = "WARN ";	break;
			case LogLevel::Error:	label = "ERROR";	break;
			case LogLevel::Fatal:	label = "FATAL";	break;
		}

		// Write to the log file
		std::lock_guard<std::mutex> locker(mMutex);

		putTime(mLogFile);
		mLogFile << " [" << label << "]";
		mLogFile << std::hex << " 0x" << std::this_thread::get_id() << std::dec;

		if (stream.good()) {
			mLogFile << " " << stream.c_str();
		}

		mLogFile << std::endl;
	}

// Private functions
	void Logger::putTime(std::ostream& os)
	{
		static const char timeFormat[] = "%Y/%m/%d %H:%M:%S";
		using namespace std::chrono;

		system_clock::time_point now = system_clock::now();
		system_clock::duration tp = now.time_since_epoch();
		tp -= duration_cast<seconds>(tp);

		std::time_t tt = system_clock::to_time_t(now);
		os	<< std::put_time(std::localtime(&tt), timeFormat)
			<< '.' << std::setw(3) << std::setfill('0') << tp / milliseconds(1);
	}

}
