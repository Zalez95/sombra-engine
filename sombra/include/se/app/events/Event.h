#ifndef I_EVENT_H
#define I_EVENT_H

#include <ostream>

namespace se::app {

	/** The different Topics that an IEvent can have */
	enum class Topic : int
	{
		Close = 0,		// Notifies window close button pressed
		MouseMove,		// Notifies mouse movement input events
		MouseScroll,	// Notifies mouse scroll input events
		MouseButton,	// Notifies mouse button input events
		Resize,			// Notifies Window resize input events
		Key,			// Notifies Keyboard input events
		TextInput,		// Notifies new input code points
		Collision,		// Notifies collision detection events
		Camera,			// Camera change events
		Shadow,			// Shadow LightSource change events
		Application,	// Reserved for client code events
		NumTopics		// Total number of events
	};


	/** Prints the topic to the given stream
	 *
	 * @param	os the stream to print to
	 * @param	t the topic to print
	 * @return	the stream */
	constexpr std::ostream& operator<<(std::ostream& os, const Topic& t)
	{
		switch (t) {
			case Topic::Close:			return os << "Topic::Close";
			case Topic::MouseScroll:	return os << "Topic::MouseScroll";
			case Topic::MouseButton:	return os << "Topic::MouseButton";
			case Topic::Resize:			return os << "Topic::Resize";
			case Topic::Key:			return os << "Topic::Key";
			case Topic::TextInput:		return os << "Topic::TextInput";
			case Topic::Collision:		return os << "Topic::Collision";
			case Topic::Camera:			return os << "Topic::Camera";
			case Topic::Shadow:			return os << "Topic::Shadow";
			case Topic::Application:	return os << "Topic::Application";
			default:					return os;
		}
	}


	/**
	 * Class IEvent, it holds the information that we want to share with the
	 * listeners subscribed to the IEvent's topic.
	 */
	class IEvent
	{
	public:		// Functions
		/** Class destructor */
		virtual ~IEvent() = default;

		/** @return	the Topic of the IEvent */
		virtual Topic getTopic() const = 0;

		/** Appends the given IEvent formated as text to the given ostream
		 *
		 * @param	os a reference to the ostream where we want to print the
		 *			IEvent
		 * @param	e the IEvent to print
		 * @return	the input ostream */
		friend std::ostream& operator<<(std::ostream& os, const IEvent& e)
		{ e.printTo(os); return os; };
	protected:
		/** Appends the current IEvent formated as text to the given ostream
		 *
		 * @param	os a reference to the ostream where we want to print the
		 *			current IEvent */
		virtual void printTo(std::ostream& os) const = 0;
	};


	/**
	 * Class IEvent, it's the base class that all events should inherit from to
	 * be notified by the EventManager
	 */
	template <Topic t>
	class Event : public IEvent
	{
	public:		// Attributes
		/** The topic of the Event */
		static constexpr Topic kTopic = t;

	public:		// Functions
		/** Class destructor */
		virtual ~Event() = default;

		/** @return	the Topic of the IEvent */
		virtual Topic getTopic() const { return kTopic; };
	protected:
		/** Appends the current Event formated as text to the given ostream
		 *
		 * @param	os a reference to the ostream where we want to print the
		 *			current ContainerEvent */
		virtual void printTo(std::ostream& os) const override
		{ os << "{ kTopic : " << this->kTopic << " }"; };
	};

}

#endif		// I_EVENT_H
