#ifndef I_SYSTEM_H
#define I_SYSTEM_H

#include "events/EventManager.h"
#include "Entity.h"

namespace se::app {

	class EntityDatabase;


	/**
	 * Class System, it's the interface that each System must implement. A
	 * System is used for updating the Entities Components at every clock tick.
	 * Also, it can listen for Events
	 */
	class ISystem : public IEventListener
	{
	protected:	// Attributes
		/** The EntityDatabase that holds all the Entities and their
		 * Components */
		EntityDatabase& mEntityDatabase;

		/** The elapsed time since the last @see update call */
		float mDeltaTime;

	public:		// Functions
		/** Creates a new ISystem
		 *
		 * @param	entityDatabase the EntityDatabase that holds all the
		 *			Entities */
		ISystem(EntityDatabase& entityDatabase) :
			mEntityDatabase(entityDatabase), mDeltaTime(0.0f) {};

		/** Class destructor */
		virtual ~ISystem() = default;

		/** Notifies the ISystem of the given event
		 *
		 * @param	event the IEvent to notify
		 * @note	you must not send events from code that handles them, nor
		 *			remove/add listeners to the manager */
		virtual void notify(const IEvent& /*event*/) override {};

		/** Function that the EntityDatabase will call when an Entity is
		 * added
		 *
		 * @param	entity the new Entity */
		virtual void onNewEntity(Entity /*entity*/) {};

		/** Function that the EntityDatabase will call when an Entity is
		 * removed
		 *
		 * @param	entity the Entity to remove */
		virtual void onRemoveEntity(Entity /*entity*/) {};

		/** Sets the delta time of the ISystem
		 *
		 * @param	deltaTime the elapsed time since the last @see update
		 *			call */
		void setDeltaTime(float deltaTime) { mDeltaTime = deltaTime; };

		/** Function called every clock tick */
		virtual void update() {};
	};

}

#endif		// I_SYSTEM_H
