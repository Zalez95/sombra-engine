#ifndef SCRIPT_SYSTEM_H
#define SCRIPT_SYSTEM_H

#include "ISystem.h"

namespace se::app {

	class Application;


	/**
	 * Class ScriptSystem, it's a System used for updating the scripts of the
	 * Entities
	 */
	class ScriptSystem : public ISystem
	{
	public:		// Functions
		/** Creates a new ScriptSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		ScriptSystem(Application& application);

		/** Class destructor */
		virtual ~ScriptSystem();

		/** @copydoc ISystem::onNewEntity(Entity) */
		virtual void onNewEntity(Entity entity);

		/** @copydoc ISystem::onRemoveEntity(Entity) */
		virtual void onRemoveEntity(Entity entity);

		/** Updates the scripts of the Entities */
		virtual void update() override;
	};

}

#endif		// SCRIPT_SYSTEM_H
