#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <queue>
#include <functional>

namespace se::utils {

	/**
	 * Class StateMachine, it's used for handling transitions between states
	 * and queueing the events that produce them, and also for notifying when
	 * they happen with callback functions
	 */
	class StateMachine
	{
	public:		// Nested types
		using State = int;
		using Event = int;

		/** Holds a transition from one state to another */
		struct Transition
		{
			/** The initial state */
			State origin;

			/** The event that triggered the state change */
			Event event;

			/** The final state */
			State destination;

			/** The function to call when the transition happens */
			std::function<void()> callback;
		};

	private:	// Attributes
		/** The transition table of the StateMachine */
		const Transition* mTransitionTable;

		/** The number of transtions in @see mTransitionTable */
		std::size_t mNumTransitions;

		/** The current State of the StateMachine */
		State mCurrentState;

		/** A queue with the submmitted Events to handle */
		std::queue<Event> mEventQueue;

	public:		// Functions
		/** Creates a new StateMachine
		 *
		 * @param	transitionTable a pointer to the transition table of the
		 *			StateMachine
		 * @param	numTransitions the number of transitions in the
		 *			@see transitionTable
		 * @param	initialState the initial state of the StateMachine */
		StateMachine(
			const Transition* transitionTable, std::size_t numTransitions,
			State initialState
		);

		/** Submits the given event to the StateMachine
		 *
		 * @param	event the Event to submit */
		void submitEvent(Event event);

		/** Handles the events submitted to the StateMachine, updating the
		 * current state of the StateMachine based on its transition table and
		 * calling their respective callback functions */
		void handleEvents();
	};

}

#endif		// STATE_MACHINE_H
