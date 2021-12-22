#ifndef SCRIPT_SYSTEM_H
#define SCRIPT_SYSTEM_H

#include "events/KeyEvent.h"
#include "events/MouseEvents.h"
#include <se/app/events/ResizeEvent.h>
#include <se/app/events/ScriptEvent.h>
#include "ECS.h"
#include "ScriptComponent.h"

namespace sol { class state; }

namespace se::app {

	class Application;


	/**
	 * Class ScriptSystem, it's a System used for updating the scripts of the
	 * Entities
	 */
	class ScriptSystem : public ISystem
	{
	private:	// Nested types
		using ScriptRef = Repository::ResourceRef<Script>;

		/** Struct ScriptData, holds all the data needed for handling
		 * different scripts languages */
		struct ScriptData
		{
			ScriptRef script;
			std::size_t userCount = 0;
		};

	private:	// Attributes
		/** The Application that holds the ScriptSystem */
		Application& mApplication;

		/** Holds all the user shared state between the scripts */
		ScriptSharedState mScriptSharedState;

		/** Holds the data of each script */
		utils::PackedVector<ScriptData> mScriptsData;

		/** The lua state used by the LUA Scripts */
		sol::state* mLuaState = nullptr;

	public:		// Functions
		/** Creates a new ScriptSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		ScriptSystem(Application& application);

		/** Class destructor */
		virtual ~ScriptSystem();

		/** @copydoc ISystem::notify(const IEvent&) */
		virtual bool notify(const IEvent& event) override;

		/** @copydoc ISystem::onNewComponent(Entity, const EntityDatabase::ComponentMask&) */
		virtual void onNewComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override
		{ tryCallC(&ScriptSystem::onNewScript, entity, mask); };

		/** @copydoc ISystem::onRemoveComponent(Entity, const EntityDatabase::ComponentMask&) */
		virtual void onRemoveComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override
		{ tryCallC(&ScriptSystem::onRemoveScript, entity, mask); };

		/** Updates the scripts of the Entities */
		virtual void update() override;
	private:
		/** Function called when a ScriptComponent is added to an Entity
		 *
		 * @param	entity the Entity that holds the ScriptComponent
		 * @param	script a pointer to the new ScriptComponent */
		void onNewScript(Entity entity, ScriptComponent* script);

		/** Function called when a ScriptComponent is going to be removed from
		 * an Entity
		 *
		 * @param	entity the Entity that holds the ScriptComponent
		 * @param	script a pointer to the ScriptComponent that is going to be
		 *			removed */
		void onRemoveScript(Entity entity, ScriptComponent* script);

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

		/** Adds a user to the given script
		 *
		 * @param	script the script to add a user */
		void addUser(const ScriptRef& script);

		/** Removes a user from the given script
		 *
		 * @param	script the script to remove a user */
		void removeUser(const ScriptRef& script);
	};

}

#endif		// SCRIPT_SYSTEM_H
