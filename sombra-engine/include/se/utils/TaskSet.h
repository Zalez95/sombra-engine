#ifndef TASK_SET_H
#define TASK_SET_H

#include "TaskManager.h"

namespace se::utils {

	/**
	 * Class SubTaskSet, it's a wrapper used for interacting with the
	 * TaskManager. It allows to create new Tasks dynamically and recursivelly
	 * from the inside of other Tasks and SubTaskSets.
	 */
	class SubTaskSet
	{
	protected:	// Nested types
		using FuncTask = std::function<void()>;
		using FuncSTS = std::function<void(SubTaskSet&)>;

	protected:	// Attributes
		/** A reference to the TaskManager used for creating and running the
		 * Tasks */
		TaskManager& mTaskManager;

		/** All the Tasks added to the SubTaskSet */
		std::vector<TaskId> mTasks;

		/** All the SubTaskSets added to the SubTaskSet */
		std::vector<SubTaskSet> mSubTaskSets;

		/** The id of the Task that is going to be executed prior to all the
		 * set tasks */
		TaskId mInitialTaskId;

		/** The id of the Task that is going to be executed after all the
		 * set tasks */
		TaskId mFinalTaskId;

		/** If @see mTasks have to be executed prior to @see mFinalTask,
		 * otherwise @see mFinalTask will be executed only after
		 * @see mInitialTask */
		bool mJoinTasks;

	public:		// Functions
		/** Creates a new SubTaskSet
		 *
		 * @param	taskManager a reference to the TaskManager used for
		 *			creating and running the Tasks
		 * @param	initialFunction the function to call in the initial task of
		 *			the Set
		 * @param	finalFunction the function to call in the final task of
		 *			the Set
		 * @param	join if the tasks added to the set must be executed prior
		 *			to the final task or not */
		SubTaskSet(
			TaskManager& taskManager,
			const FuncSTS& initialFunction, const FuncSTS& finalFunction,
			bool join
		);

		/** Class destructor */
		virtual ~SubTaskSet() = default;

		/** Creates and adds a new Task to the current SubTaskSet
		 *
		 * @param	function the function to execute in the new Task
		 * @return	the id of the new Task, -1 if it couldn't be created */
		TaskId createTask(const FuncTask& function);

		/** Creates and adds a new SubTaskSet to the current SubTaskSet
		 *
		 * @param	function the function to execute when SubTaskSet is created
		 * @param	join if the tasks added to the new SubTaskSet must be
		 *			executed prior to its final task or not
		 * @return	a reference to the SubTaskSet */
		SubTaskSet& createSubTaskSet(const FuncSTS& function, bool join = true);

		/** Adds a dependency between the given Tasks
		 *
		 * @param	taskId1 the Task that will depend on the other one
		 * @param	taskId2 the other Task */
		void depends(TaskId taskId1, TaskId taskId2);

		/** Adds a dependency between the given SubTaskSet and the given Task
		 *
		 * @param	subSet1 the SubTaskSet that will depend on the given Task
		 * @param	taskId2 the Task */
		void depends(const SubTaskSet& subSet1, TaskId taskId2);

		/** Adds a dependency between the given Task and the given SubTaskSet
		 *
		 * @param	taskId1 the Task that will depend on the given SubTaskSet
		 * @param	subSet2 the SubTaskSet */
		void depends(TaskId taskId1, const SubTaskSet& subSet2);

		/** Adds a dependency between the given SubTaskSets
		 *
		 * @param	subSet1 the SubTaskSet that will depend on the other one
		 * @param	subSet2 the other SubTaskSet */
		void depends(const SubTaskSet& subSet1, const SubTaskSet& subSet2);
	protected:
		/** Submits the Tasks of the SubTaskSet to the TaskManager */
		void submitSubTaskSetTasks();

		/** Submits all the Tasks added to the SubTaskSet to the TaskManager */
		void submitCreatedTasks();
	};


	/**
	 * Class TaskSet, it's a wrapper used for interacting with the
	 * TaskManager. It allows to create new Tasks dynamically and recursivelly
	 * from the inside of other Tasks and SubTaskSets. It also allows to the
	 * caller thread to wait until all the Tasks added to the TaskSet and its
	 * SubTaskSets have finished.
	 */
	class TaskSet : public SubTaskSet
	{
	private:
		/** Mutex used in conjunction with @see mCV for waiting */
		std::mutex mMutex;

		/** The condition variable used for waiting until all the tasks
		 * finish */
		std::condition_variable mCV;

	public:
		/** Creates a new TaskSet
		 *
		 * @param	taskManager a reference to the TaskManager used for
		 *			creating and running the Tasks */
		TaskSet(TaskManager& taskManager);

		/** Submits all the Tasks to the TaskManager */
		void submit();

		/** Submits all the Tasks to the TaskManager and waits until all of
		 * them are finished */
		void submitAndWait();
	};

}

#endif		// TASK_SET_H
