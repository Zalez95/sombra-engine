#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include <array>
#include <vector>
#include <functional>
#include "Event.h"

namespace se::app
{

	/**
	 * Class IEventListener, each object that needs to get notified of IEvents
	 * must implement this class.
	 */
	class IEventListener
	{
	public:		// Functions
		/** Class destructor */
		virtual ~IEventListener() = default;

		/** Notifies the IEventListener of the given event
		 *
		 * @param	event the IEvent to notify
		 * @note	you must not send events from code that handles them, nor
		 *			remove/add listeners to the manager */
		virtual void notify(const IEvent& event) = 0;
	protected:
		/** Tries to call the given event handler function with the correct
		 * event type
		 *
		 * @param	eventHandler a pointer to the function to call
		 * @param	event the IEvent to call the function with */
		template <typename C, typename E>
		void tryCall(void(C::*eventHandler)(const E&), const IEvent& event)
		{
			if (event.getTopic() == E::kTopic) {
				C* thisC = static_cast<C*>(this);
				(thisC->*eventHandler)( static_cast<const E&>(event) );
			}
		}
	};


	/**
	 * Class EventManager, it's used for notifying the published IEvents to the
	 * IEventListeners subscribed to the Topic of the IEvent
	 */
	class EventManager
	{
	private:	// Nested types
		using ListenerVector = std::vector<IEventListener*>;

	private:	// Attributes
		/** The number of available topics */
		static constexpr int NumTopics = static_cast<int>(Topic::NumTopics);

		/** The pointers to the listeners of each Topic */
		std::array<ListenerVector, NumTopics> mListenersPerTopic;

	public:		// Functions
		/** Subscribes the given IEventListener to the given topic, so the
		 * published events in the EventManager with that topic will be
		 * notified to the listener
		 *
		 * @param	eventListener a pointer to the IEventListener to notify
		 * @param	topic the topic to subscribe the eventListener */
		void subscribe(IEventListener* eventListener, Topic topic);

		/** Unsubscribes the given IEventListener from the given topic, so the
		 * published events in the EventManager with that topic will no longer
		 * be notified to the listener
		 *
		 * @param	eventListener a pointer to the IEventListener to unsubscribe
		 * @param	topic the topic to unsubscribe */
		void unsubscribe(IEventListener* eventListener, Topic topic);

		/** Publishes the given event to the EventManager, so the
		 * IEventListeners subscribed to the same topics than the event will be
		 * notified of it
		 *
		 * @param	event a pointer to the IEvent to notify */
		void publish(IEvent* event);
	};

}

#endif		// EVENT_MANAGER_H
