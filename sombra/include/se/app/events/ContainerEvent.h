#ifndef CONTAINER_EVENT_H
#define CONTAINER_EVENT_H

#include "Event.h"

namespace se::app {

	/**
	 * Class ContainerEvent, its an event used for notifying an object of
	 * type @tparam T with the given @tparam t Topic
	 */
	template <Topic t, typename T>
	class ContainerEvent : public Event<t>
	{
	private:	// Attributes
		/** The value to notify */
		T mValue;

	public:		// Functions
		/** Creates a new ContainerEvent
		 *
		 * @param	value the value to notify */
		ContainerEvent(const T& value) : mValue(value) {};

		/** @return	the value */
		const T& getValue() const { return mValue; };

		/** @return	the value */
		T& getValue() { return mValue; };
	protected:
		/** @copydoc Event::printTo() */
		virtual void printTo(std::ostream& os) const override
		{
			os	<< "{ kTopic : " << this->kTopic
				<< ", mValue : " << mValue << " }";
		};
	};

}

#endif		// CONTAINER_EVENT_H
