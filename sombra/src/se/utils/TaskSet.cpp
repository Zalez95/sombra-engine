#include "se/utils/TaskSet.h"
#include "se/utils/Log.h"

namespace se::utils {

	SubTaskSet::SubTaskSet(
		TaskManager& taskManager, const FuncSTS& initialFunction, const FuncSTS& finalFunction, bool join
	) : mTaskManager(taskManager), mInitialTaskId(-1), mFinalTaskId(-1), mJoinTasks(join)
	{
		mTasks.reserve(mTaskManager.getMaxTasks());
		mSubTaskSets.reserve(mTaskManager.getMaxTasks() / 2);

		mInitialTaskId = (initialFunction)?
			mTaskManager.create([this, initialFunction]() { initialFunction(*this); submitCreatedTasks(); }) :
			mTaskManager.create([this]() { submitCreatedTasks(); });
		mFinalTaskId = finalFunction?
			mTaskManager.create([this, finalFunction]() { finalFunction(*this); }) :
			mTaskManager.create([]() {});
		mTaskManager.addDependency(mFinalTaskId, mInitialTaskId);

		SOMBRA_TRACE_LOG << "Set[" << this << "] Created SubTaskSet with InitialTask " << mInitialTaskId
			<< ", FinalTask " << mFinalTaskId << " and join " << mJoinTasks;
	}


	TaskId SubTaskSet::createTask(const FuncTask& function)
	{
		TaskId taskId = mTaskManager.create(function);
		if (taskId >= 0) {
			mTasks.push_back(taskId);

			if (mJoinTasks) {
				mTaskManager.addDependency(mFinalTaskId, taskId);
			}
		}

		SOMBRA_TRACE_LOG << "Set[" << this << "] Added task " << taskId;
		return taskId;
	}


	SubTaskSet& SubTaskSet::createSubTaskSet(const FuncSTS& function, bool join)
	{
		SubTaskSet& ret = mSubTaskSets.emplace_back(mTaskManager, function, FuncSTS(), join);
		if (mJoinTasks) {
			mTaskManager.addDependency(mFinalTaskId, ret.mFinalTaskId);
		}

		SOMBRA_TRACE_LOG << "Set[" << this << "] Added SubTaskSet " << &ret;
		return ret;
	}


	void SubTaskSet::depends(TaskId taskId1, TaskId taskId2)
	{
		mTaskManager.addDependency(taskId1, taskId2);
	}


	void SubTaskSet::depends(const SubTaskSet& subSet1, TaskId taskId2)
	{
		mTaskManager.addDependency(subSet1.mInitialTaskId, taskId2);
	}


	void SubTaskSet::depends(TaskId taskId1, const SubTaskSet& subSet2)
	{
		mTaskManager.addDependency(taskId1, subSet2.mFinalTaskId);
	}


	void SubTaskSet::depends(const SubTaskSet& subSet1, const SubTaskSet& subSet2)
	{
		mTaskManager.addDependency(subSet1.mInitialTaskId, subSet2.mFinalTaskId);
	}


	void SubTaskSet::submitSubTaskSetTasks()
	{
		SOMBRA_TRACE_LOG << "Set[" << this << "] Start";

		mTaskManager.submit(mInitialTaskId);
		mTaskManager.submit(mFinalTaskId);

		SOMBRA_TRACE_LOG << "Set[" << this << "] End";
	}


	void SubTaskSet::submitCreatedTasks()
	{
		SOMBRA_TRACE_LOG << "Set[" << this << "] Start";

		for (auto& taskId : mTasks) {
			mTaskManager.submit(taskId);
		}
		for (auto& subSet : mSubTaskSets) {
			subSet.submitSubTaskSetTasks();
		}

		SOMBRA_TRACE_LOG << "Set[" << this << "] End";
	}


	TaskSet::TaskSet(TaskManager& taskManager) :
		SubTaskSet(
			taskManager,
			FuncSTS(),
			[](SubTaskSet& set) { dynamic_cast<TaskSet*>(&set)->mCV.notify_all(); },
			true
		) {}


	void TaskSet::submit()
	{
		SubTaskSet::submitSubTaskSetTasks();
	}


	void TaskSet::submitAndWait()
	{
		SOMBRA_TRACE_LOG << "Set[" << this << "] Start";

		SubTaskSet::submitSubTaskSetTasks();

		std::unique_lock<std::mutex> lock(mMutex);
		mCV.wait(lock);

		SOMBRA_TRACE_LOG << "Set[" << this << "] End";
	}

}
