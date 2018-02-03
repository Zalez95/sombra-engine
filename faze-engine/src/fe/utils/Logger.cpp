#include "fe/utils/Logger.h"
#include <ctime>
#include <chrono>

namespace fe { namespace utils {

	Logger::~Logger()
	{
		mLogFile.close();
	}


	void Logger::writeLog(LogType type, const std::string& text)
	{
		static Logger mInstance(LOG_PATH);		// Create the static instance
		mInstance.write(type, text);
	}

	// Private functions
	Logger::Logger(const std::string& logPath) : mLogFile(logPath, std::ios::app) {}


	void Logger::write(LogType type, const std::string& text)
	{
		// Lock for writing in the log file and formatting the time
		// (tm is a static variable, so it isn't thread safe)
		std::lock_guard<std::mutex> locker(mMutex);
		time_t t = time(0);

		// Write the type of the text
		switch (type)
		{
		case LogType::WARNING:
			mLogFile << "[WARNING]\t";
			break;
		case LogType::ERROR:
			mLogFile << "[ERROR]\t";
			break;
		case LogType::DEBUG:
			mLogFile << "[DEBUG]\t";
			break;
		}

		// Write current time
		tm* now = localtime(&t);
		mLogFile	<< '['
					<< now->tm_mday << '/'
					<< now->tm_mon + 1 << '/'
					<< now->tm_year + 1900 << '\t'
					<< now->tm_hour << ':'
					<< now->tm_min << ':'
					<< now->tm_sec << "]\t"
					<< text << "\n";
	}

}}
