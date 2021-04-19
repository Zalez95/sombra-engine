#include <algorithm>
#include "se/utils/TaskManager.h"
#include "se/utils/Log.h"

namespace se::utils {

	TaskManager::TaskManager(int maxTasks, int numThreads) :
		mTasks(maxTasks), mThreads(numThreads, nullptr), mEnd(false)
	{
		SOMBRA_INFO_LOG << "Creating TaskManager with up to " << maxTasks
			<< " tasks and " << numThreads << " threads";

		for (auto& task : mTasks) {
			task.dependentTasks.reserve(maxTasks);
		}

		for (auto& th : mThreads) {
			th = new std::thread([this]() { thRun(); });
		}
	}


	TaskManager::~TaskManager()
	{
		SOMBRA_INFO_LOG << "Destroying TaskManager";

		std::unique_lock lck(mMutex);
		mEnd = true;
		lck.unlock();
		mCV.notify_all();

		for (auto& th : mThreads) {
			if (th && th->joinable()) {
				th->join();
				delete th;
			}
		}

		SOMBRA_INFO_LOG << "TaskManager destroyed";
	}


	TaskId TaskManager::create(const std::function<void()>& function)
	{
		TaskId taskId = -1;

		if (function) {
			for (TaskId taskId2 = 0; taskId2 < static_cast<TaskId>(mTasks.size()); ++taskId2) {
				while (mTasks[taskId2].lock.test_and_set(std::memory_order_acquire));

				if (mTasks[taskId2].state == TaskState::Released) {
					mTasks[taskId2].state = TaskState::Created;
					mTasks[taskId2].function = function;
					mTasks[taskId2].lock.clear(std::memory_order_release);

					taskId = taskId2;
					break;
				}

				mTasks[taskId2].lock.clear(std::memory_order_release);
			}

			if (taskId >= 0) {
				SOMBRA_TRACE_LOG << "Created Task " << taskId;
			}
			else {
				SOMBRA_WARN_LOG << "Can't create more tasks";
			}
		}
		else {
			SOMBRA_WARN_LOG << "Not callable function";
		}

		return taskId;
	}


	void TaskManager::addDependency(TaskId taskId1, TaskId taskId2)
	{
		while (mTasks[taskId1].lock.test_and_set(std::memory_order_acquire));
		while (mTasks[taskId2].lock.test_and_set(std::memory_order_acquire));

		auto itDependent = std::find(
			mTasks[taskId2].dependentTasks.begin(), mTasks[taskId2].dependentTasks.end(),
			taskId1
		);
		if (((mTasks[taskId1].state == TaskState::Created) || (mTasks[taskId1].state == TaskState::Submitted))
			&& ((mTasks[taskId2].state == TaskState::Created) || (mTasks[taskId2].state == TaskState::Submitted))
			&& (itDependent == mTasks[taskId2].dependentTasks.end())
		) {
			mTasks[taskId1].remainingTasks++;
			mTasks[taskId2].dependentTasks.push_back(taskId1);
			SOMBRA_TRACE_LOG << "Added dependency between " << taskId1 << " and " << taskId2;
		}
		else {
			SOMBRA_WARN_LOG << "Can't add dependency between " << taskId1 << " and " << taskId2;
		}

		mTasks[taskId2].lock.clear(std::memory_order_release);
		mTasks[taskId1].lock.clear(std::memory_order_release);
	}


	void TaskManager::submit(TaskId taskId)
	{
		while (mTasks[taskId].lock.test_and_set(std::memory_order_acquire));
		if (mTasks[taskId].state == TaskState::Created) {
			mTasks[taskId].state = TaskState::Submitted;
			mTasks[taskId].lock.clear(std::memory_order_release);

			// Push the taskId to mWorkingQueue and notify so it can be executed
			{
				std::unique_lock lck(mMutex);
				mWorkingQueue.push_back(taskId);
			}
			mCV.notify_one();

			SOMBRA_TRACE_LOG << "Submitted Task " << taskId;
		}
		else {
			mTasks[taskId].lock.clear(std::memory_order_release);
			SOMBRA_WARN_LOG << "Can't submit Task " << taskId;
		}
	}

// Private functions
	void TaskManager::thRun()
	{
		SOMBRA_INFO_LOG << "Thread start";

		std::unique_lock lck(mMutex);
		while (!mEnd) {
			TaskId taskId = getTaskId();
			if (taskId >= 0) {
				lck.unlock();

				SOMBRA_TRACE_LOG << "Executing task " << taskId;
				mTasks[taskId].function();
				releaseTask(taskId);
				SOMBRA_TRACE_LOG << "Released task " << taskId;

				lck.lock();
			}
			else {
				mCV.wait(lck);
			}
		}

		SOMBRA_INFO_LOG << "Thread end";
	}


	TaskId TaskManager::getTaskId()
	{
		TaskId taskId = -1;

		// Find a Task in the Queue that has 0 remaining tasks and is in a
		// Submitted state.
		for (std::size_t i = 0; i < mWorkingQueue.size();) {
			TaskId taskId2 = mWorkingQueue[i];

			while (mTasks[taskId2].lock.test_and_set(std::memory_order_acquire));

			if ((i == 0) && (mTasks[taskId2].state == TaskState::Released)) {
				mWorkingQueue.pop_front();
			}
			else if ((mTasks[taskId2].state == TaskState::Submitted) && (mTasks[taskId2].remainingTasks == 0)) {
				mTasks[taskId2].state = TaskState::Running;
				mTasks[taskId2].lock.clear(std::memory_order_release);

				taskId = taskId2;
				break;
			}
			else {
				i++;
			}

			mTasks[taskId2].lock.clear(std::memory_order_release);
		}

		return taskId;
	}


	void TaskManager::releaseTask(TaskId taskId)
	{
		while (mTasks[taskId].lock.test_and_set(std::memory_order_acquire));

		mTasks[taskId].state = TaskState::Released;

		// Decrement the dependentTasks' remainingTasks
		for (TaskId dependentTaskId : mTasks[taskId].dependentTasks) {
			while (mTasks[dependentTaskId].lock.test_and_set(std::memory_order_acquire));
			mTasks[dependentTaskId].remainingTasks--;
			mTasks[dependentTaskId].lock.clear(std::memory_order_release);
		}
		mTasks[taskId].dependentTasks.clear();

		mTasks[taskId].lock.clear(std::memory_order_release);

		// Notify so the dependent tasks can be executed
		mCV.notify_all();
	}

}
