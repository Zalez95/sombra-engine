#ifndef I_EVENT_H
#define I_EVENT_H

namespace se::app {

	/** The different Topics that an IEvent can have */
	enum class Topic : int
	{
		Movement,
		NumTopics
	};


	/**
	 * Class IEvent, the base IEvent that all events should inherit from.
	 * @note	derived events should also have a kTopic public constant
	 *			property with the Topic of the IEvent
	 */
	class IEvent
	{
	public:		// Functions
		/** Class destructor */
		virtual ~IEvent() {};

		/** @return	the Topic of the IEvent */
		virtual Topic getTopic() const = 0;
	};

}

#endif		// I_EVENT_H
