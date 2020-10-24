#ifndef PROFILER_H
#define PROFILER_H

#include <mutex>
#include <thread>
#include <chrono>
#include <fstream>

namespace se::utils {

	class Profiler
	{
	private:
		std::ofstream mFileStream;
		std::size_t mCountProfiles;
		std::mutex mMutex;
	public:
		static Profiler& getInstance()
		{
			static Profiler instance("profile.json");
			return instance;
		}

		~Profiler()
		{
			mFileStream << "]}";
			mFileStream.flush();
		};

		void write(const char* name, std::size_t threadId, long long start, long long end)
		{
			std::unique_lock lock(mMutex);

			if (mCountProfiles > 0) {
				mFileStream << ",";
			}

			mFileStream << "{"
				<< "\"cat\":\"function\","
				<< "\"dur\":" << (end - start) << ","
				<< "\"name\":\"" << name << "\","
				<< "\"ph\":\"X\","
				<< "\"pid\":0,"
				<< "\"tid\":" << threadId << ","
				<< "\"ts\":" << start
				<< "}";

			mCountProfiles++;
		};
	private:
		Profiler(const char* filePath) : mFileStream(filePath), mCountProfiles(0)
		{
			mFileStream << "{\"otherData\":{},\"traceEvents\":[";
		};
	};


	class TimeGuard
	{
	private:
		const char* mName;
		std::chrono::time_point<std::chrono::high_resolution_clock> mStartTP;

	public:
		TimeGuard(const char* name) : mName(name)
		{
			mStartTP = std::chrono::high_resolution_clock::now();
		};

		~TimeGuard()
		{
			auto endTP = std::chrono::high_resolution_clock::now();
			std::size_t threadId = std::hash<std::thread::id>{}(std::this_thread::get_id());

			long long start = std::chrono::time_point_cast<std::chrono::microseconds>(mStartTP).time_since_epoch().count();
			long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTP).time_since_epoch().count();

			Profiler::getInstance().write(mName, threadId, start, end);
		};
	};

}

#endif		// PROFILER_H
