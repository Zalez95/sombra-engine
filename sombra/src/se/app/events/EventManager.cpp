#include "se/app/events/EventManager.h"
#include "se/utils/Log.h"

namespace se::app {

	EventManager& EventManager::subscribe(IEventListener* eventListener, Topic topic)
	{
		if (eventListener) {
			int iTopic = static_cast<int>(topic);
			mListenersPerTopic[iTopic].push_back(eventListener);

			SOMBRA_INFO_LOG << "Subscribed IEventListener " << eventListener << " to " << topic
				<< " at index " << mListenersPerTopic[iTopic].size() - 1;
		}

		return *this;
	}


	EventManager& EventManager::unsubscribe(IEventListener* eventListener, Topic topic)
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

		return *this;
	}


	EventManager& EventManager::publish(std::unique_ptr<IEvent> event)
	{
		if (event) {
			Topic topic = event->getTopic();
			for (IEventListener* listener : mListenersPerTopic[static_cast<int>(topic)]) {
				if (!listener->notify(*event)) {
					SOMBRA_WARN_LOG << "IEventListener " << listener << " is subscribed to " << topic
						<< " but doesn't handle it's events";
				}
			}
		}

		return *this;
	}

}
