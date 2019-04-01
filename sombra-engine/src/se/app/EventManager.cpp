#include "se/app/EventManager.h"

namespace se::app
{

	void EventManager::subscribe(IEventListener* eventListener, Topic topic)
	{
		if (eventListener) {
			int iTopic = static_cast<int>(topic);
			mListenersPerTopic[iTopic].push_back(eventListener);
		}
	}


	void EventManager::unsubscribe(IEventListener* eventListener, Topic topic)
	{
		if (eventListener) {
			int iTopic = static_cast<int>(topic);
			mListenersPerTopic[iTopic].erase(
				std::remove(mListenersPerTopic[iTopic].begin(), mListenersPerTopic[iTopic].end(), eventListener),
				mListenersPerTopic[iTopic].end()
			);
		}
	}


	void EventManager::publish(IEvent* event)
	{
		if (event) {
			Topic topic = event->getTopic();
			for (IEventListener* listener : mListenersPerTopic[static_cast<int>(topic)]) {
				listener->notify(*event);
			}
			delete event;
		}
	}

}
