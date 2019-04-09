#ifndef I_EVENT_H
#define I_EVENT_H

#include <ostream>

namespace se::app {

	/** The different Topics that an IEvent can have */
	enum class Topic : int
	{
		Collision,
		NumTopics
	};


	constexpr std::ostream& operator<<(std::ostream& os, const Topic& t)
	{
		switch (t) {
			case Topic::Collision:	return os << "Topic::Collision";
			default:				return os;
		}
	}


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
		virtual void printTo(std::ostream& os) const = 0 ;
	};

}

#endif		// I_EVENT_H
