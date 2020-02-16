#include "se/utils/StateMachine.h"

namespace se::utils {

	StateMachine::StateMachine(const Transition* transitionTable, std::size_t numTransitions, State initialState) :
		mTransitionTable(transitionTable), mNumTransitions(numTransitions), mCurrentState(initialState) {}


	void StateMachine::submitEvent(Event event)
	{
		mEventQueue.push(event);
	}


	void StateMachine::handleEvents()
	{
		while (!mEventQueue.empty()) {
			Event event = mEventQueue.back();
			mEventQueue.pop();

			for (std::size_t i = 0; i < mNumTransitions; ++i) {
				if ((mTransitionTable[i].origin == mCurrentState) && (mTransitionTable[i].event == event)) {
					mCurrentState = mTransitionTable[i].destination;
					mTransitionTable[i].callback();
					break;
				}
			}
		}
	}

}
