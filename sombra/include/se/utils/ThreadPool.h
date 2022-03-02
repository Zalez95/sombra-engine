#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <deque>
#include <memory>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <future>
#include <chrono>

namespace se::utils {

	template<typename T>
	bool is_ready(std::future<T> const& f)
	{ return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready; }


	/**
	 * Class ThreadPool, it's used for executing tasks asynchronously without
	 * the overhead of creating new threads. The number of worker threads is set
	 * on construction and it doesn't change. If all the threads are busy the
	 * new tasks will be queued until a thread is idle.
	 */
	class ThreadPool
	{
	private:	// Nested types
		/** All the threads of the ThreadPool */
		std::thread** mThreads;

		/** The number of threads in @see mThreads */
		std::size_t mNumThreads;

		/** A flag used for stoping the threads */
		bool mStop;

		/** The FIFO queue used for submiting the tasks to the threads */
		std::deque<std::function<void()>> mTasksQueue;

		/** The mutex used for protecting @see mStop and @see mTaskQueue */
		std::mutex mMutex;

		/** The condition variable used for notifying the threads */
		std::condition_variable mCV;

	public:		// Functions
		/** Creates a new ThreadPool
		 *
		 * @param	numThreads the number of execution threads of the
		 *			ThreadPool. It's value by default is the number of harware
		*			threads */
		ThreadPool(
			std::size_t numThreads = std::thread::hardware_concurrency()
		);

		/** Class destructor. It stops all the threads */
		~ThreadPool();

		/** Executes the given function asynchronously
		 *
		 * @param	function the function to execute. It will be submitted to
		 *			the tasks queue and when a thread is idle it will run it
		 * @return	a future object with the result of the function */
		template <typename F>
		std::future<std::invoke_result_t<F>> async(F&& function);
	private:
		/** The function that will run each of the threads */
		void thRun();
	};


	template <typename F>
	std::future<std::invoke_result_t<F>> ThreadPool::async(F&& function)
	{
		using TaskType = std::packaged_task<std::invoke_result_t<F>()>;

		auto task = std::make_shared<TaskType>(std::forward<F>(function));
		auto future = task->get_future();

		{
			std::scoped_lock lock(mMutex);
			mTasksQueue.push_back([task = std::move(task)]() { (*task)(); });
		}
		mCV.notify_one();

		return future;
	}

}

#endif		// THREAD_POOL_H
