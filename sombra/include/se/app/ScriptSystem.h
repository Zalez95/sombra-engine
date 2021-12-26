#ifndef SCRIPT_SYSTEM_H
#define SCRIPT_SYSTEM_H

#include "events/KeyEvent.h"
#include "events/MouseEvents.h"
#include "events/ResizeEvent.h"
#include "events/ScriptEvent.h"
#include "events/EventManager.h"
#include "ECS.h"
#include "ScriptComponent.h"

namespace se::app {

	class Application;


	/**
	 * Class ScriptSystem, it's a System used for updating the scripts of the
	 * Entities
	 */
	class ScriptSystem : public ISystem, public IEventListener
	{
	public:		// Attributes
		/** The Application that holds the ScriptSystem */
		Application& mApplication;

		/** Holds all the user shared state between the scripts */
		ScriptSharedState mScriptSharedState;

	public:		// Functions
		/** Creates a new ScriptSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		ScriptSystem(Application& application);

		/** Class destructor */
		virtual ~ScriptSystem();

		/** @copydoc IEventListener::notify(const IEvent&) */
		virtual bool notify(const IEvent& event) override;

		/** @copydoc ISystem::onNewComponent(Entity, const EntityDatabase::ComponentMask&, EntityDatabase::Query&) */
		virtual void onNewComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask,
			EntityDatabase::Query& query
		) override
		{ tryCallC(&ScriptSystem::onNewScript, entity, mask, query); };

		/** @copydoc ISystem::onRemoveComponent(Entity, const EntityDatabase::ComponentMask&, EntityDatabase::Query&) */
		virtual void onRemoveComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask,
			EntityDatabase::Query& query
		) override
		{ tryCallC(&ScriptSystem::onRemoveScript, entity, mask, query); };

		/** Updates the scripts of the Entities */
		virtual void update() override;
	private:
		/** Function called when a ScriptComponent is added to an Entity
		 *
		 * @param	entity the Entity that holds the ScriptComponent
		 * @param	script a pointer to the new ScriptComponent
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components */
		void onNewScript(
			Entity entity, ScriptComponent* script, EntityDatabase::Query& query
		);

		/** Function called when a ScriptComponent is going to be removed from
		 * an Entity
		 *
		 * @param	entity the Entity that holds the ScriptComponent
		 * @param	script a pointer to the ScriptComponent that is going to be
		 *			removed
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components */
		void onRemoveScript(
			Entity entity, ScriptComponent* script, EntityDatabase::Query& query
		);

		/** Handles the given event
		 *
		 * @param	event the KeyEvent to handle */
		void onKeyEvent(const KeyEvent& event);

		/** Handles the given event
		 *
		 * @param	event the MouseMoveEvent to handle */
		void onMouseMoveEvent(const MouseMoveEvent& event);

		/** Handles the given event
		 *
		 * @param	event the MouseScrollEvent to handle */
		void onMouseScrollEvent(const MouseScrollEvent& event);

		/** Handles the given event
		 *
		 * @param	event the MouseButtonEvent to handle */
		void onMouseButtonEvent(const MouseButtonEvent& event);

		/** Handles the given event
		 *
		 * @param	event the WindowResizeEvent to handle */
		void onWindowResizeEvent(const WindowResizeEvent& event);

		/** Handles the given event
		 *
		 * @param	event the ScriptEvent to handle */
		void onScriptEvent(const ScriptEvent& event);
	};

}

#endif		// SCRIPT_SYSTEM_H
