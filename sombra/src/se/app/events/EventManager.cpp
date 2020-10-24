#include "se/app/events/EventManager.h"
#include "se/utils/Log.h"

namespace se::app {

	void EventManager::subscribe(IEventListener* eventListener, Topic topic)
	{
		if (eventListener) {
			int iTopic = static_cast<int>(topic);
			mListenersPerTopic[iTopic].push_back(eventListener);

			SOMBRA_INFO_LOG << "Subscribed IEventListener " << eventListener << " to " << topic
				<< " at index " << mListenersPerTopic[iTopic].size() - 1;
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

			SOMBRA_INFO_LOG << "Unsubscribed IEventListener " << eventListener << " from " << topic
				<< " at index " << mListenersPerTopic[iTopic].size();
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
