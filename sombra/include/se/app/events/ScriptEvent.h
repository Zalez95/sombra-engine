#ifndef SCRIPT_EVENT_H
#define SCRIPT_EVENT_H

#include "../Entity.h"
#include "Event.h"

namespace se::app {

	/**
	 * Class ScriptEvent, its an event used for notifying of a ScriptComponent
	 * update
	 */
	class ScriptEvent : public Event<Topic::Script>
	{
	public:		// Nested types
		/** The different operations to perform with the ScriptEvent */
		enum class Operation { Add, Remove };

	private:	// Attributes
		/** The Operation of the ScriptEvent */
		Operation mOperation;

		/** The Entity owner of the ScriptComponent */
		Entity mEntity;

	public:		// Functions
		/** Creates a new ScriptEvent used for notifying of a ScriptComponent
		 * update
		 *
		 * @param	operation the Operation to perform
		 * @param	entity the Entity owner of the ScriptComponent */
		ScriptEvent(Operation operation, Entity entity) :
			mOperation(operation), mEntity(entity) {};

		/** @return	the Operation to perform */
		Operation getOperation() const { return mOperation; };

		/** @return	the Entity owner of the ScriptComponent */
		Entity getEntity() const { return mEntity; };
	private:
		/** Prints the operation to the given stream
		 *
		 * @param	os the stream to print to
		 * @param	o the operation to print
		 * @return	the stream */
		friend constexpr std::ostream& operator<<(
			std::ostream& os, const Operation& o
		) {
			switch (o) {
				case Operation::Add:	return os << "Operation::Add";
				case Operation::Remove:	return os << "Operation::Remove";
				default:				return os;
			}
		};

		/** @copydoc Event::printTo() */
		virtual void printTo(std::ostream& os) const override
		{
			os	<< "{ kTopic : " << kTopic
				<< ", mOperation : " << mOperation
				<< ", mEntity : " << mEntity << " }";
		};
	};

}

#endif		// SCRIPT_EVENT_H
