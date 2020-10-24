#ifndef SCRIPT_COMPONENT_H
#define SCRIPT_COMPONENT_H

#include "EntityDatabase.h"

namespace se::app {

	/**
	 * Class ScriptComponent, it's used for changing the behaviour of an Entity
	 * at runtime
	 */
	class ScriptComponent
	{
	protected:
		/** The EntityDatabasethat holds @see mEntity */
		EntityDatabase* mEntityDatabase = nullptr;

		/** The Entity that owns the ScriptComponent */
		Entity mEntity = kNullEntity;

	public:		// Functions
		/** Class destructor */
		virtual ~ScriptComponent() {};

		/** Sets the ScriptComponent attributes */
		void setup(EntityDatabase& entityDatabase, Entity entity)
		{ mEntityDatabase = &entityDatabase; mEntity = entity; };

		/** Function called when the ScriptComponent is added to an Entity */
		virtual void onCreate() {};

		/** Function called when the ScriptComponent is removed from an
		 * Entity */
		virtual void onDestroy() {};

		/** Function called every clock tick */
		virtual void onUpdate(float /*elapsedTime*/) {};
	};

}

#endif		// SCRIPT_COMPONENT_H
