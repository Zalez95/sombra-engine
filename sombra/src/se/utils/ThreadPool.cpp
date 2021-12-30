#include "se/utils/ThreadPool.h"

namespace se::utils {

	ThreadPool::ThreadPool(std::size_t numThreads) :
		mThreads(nullptr), mNumThreads(numThreads), mStop(false)
	{
		mThreads = new std::thread*[mNumThreads];
		for (std::size_t i = 0; i < mNumThreads; ++i) {
			mThreads[i] = new std::thread([this]() { thRun(); });
		}
	}


	ThreadPool::~ThreadPool()
	{
		{
			std::scoped_lock lock(mMutex);
			mStop = true;
		}
		mCV.notify_all();

		for (std::size_t i = 0; i < mNumThreads; ++i) {
			mThreads[i]->join();
			delete mThreads[i];
		}
		delete[] mThreads;
	}


	void ThreadPool::thRun()
	{
		std::unique_lock<std::mutex> lock(mMutex);
		while (!mStop) {
			if (mTasksQueue.empty()) {
				mCV.wait(lock);
			}
			else {
				auto task = mTasksQueue.front();
				mTasksQueue.pop_front();
				lock.unlock();

				task();

				lock.lock();
			}
		}
	}

}
