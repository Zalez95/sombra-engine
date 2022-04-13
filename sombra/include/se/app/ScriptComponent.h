#ifndef SCRIPT_COMPONENT_H
#define SCRIPT_COMPONENT_H

#include <array>
#include <functional>
#include "ECS.h"
#include "Repository.h"

namespace se::app {

	class EventManager;


	/**
	 * Struct ScriptSharedState, it holds all the data accesible by the Scripts
	 */
	struct ScriptSharedState
	{
		/** Max number of buttons in the mouse */
		static constexpr int kMaxMouseButtons = 8;

		/** Max number of buttons in the keys */
		static constexpr int kMaxKeys = 512;

		/** An array with the mouse pressed keys */
		std::array<bool, kMaxMouseButtons> mouseButtons = {};

		/** An array with the keyboard pressed keys */
		std::array<bool, kMaxKeys> keys = {};

		/** The Mouse X coordinate relative to the top left corner of the
		 * window*/
		float mouseX = 0.0f;

		/** The Mouse Y coordinate relative to the top left corner of the
		 * window */
		float mouseY = 0.0f;

		/** The Mouse horizontal scroll offset */
		float scrollOffsetX = 0.0f;

		/** The Mouse vertical scroll offset */
		float scrollOffsetY = 0.0f;

		/** The width of the window */
		float windowWidth = 0.0f;

		/** The height of the window */
		float windowHeight = 0.0f;

		/** The EntityDatabase that holds the Entities */
		EntityDatabase* entityDatabase = nullptr;

		/** The EventManager used for creating events */
		EventManager* eventManager = nullptr;

		/** The elapsed time since the last update in seconds */
		float deltaTime = 0.0f;

		/** The elapsed time since the start of the Application in seconds */
		float timeSinceStart = 0.0f;
	};


	/**
	 * Class Script, it holds the functions used for changing the behaviour
	 * of an Entity at runtime
	 */
	class Script
	{
	public:		// Functions
		/** Creates a new Script */
		Script() = default;
		Script(const Script& other) = default;
		Script(Script&& other) = default;

		/** Class destructor */
		virtual ~Script() = default;

		/** Assignment operator */
		Script& operator=(const Script& other) = default;
		Script& operator=(Script&& other) = default;

		/** @return	a pointer to a copy of the current Script */
		virtual std::unique_ptr<Script> clone() const = 0;

		/** Function called when a ScriptComponent is added to an Entity
		 *
		 * @param	entity the Entity where the Component was added
		 * @param	sharedState the current user input state */
		virtual void onAdd(
			Entity /*entity*/, const ScriptSharedState& /*sharedState*/
		) {};

		/** Function called when a ScriptComponent is removed from an Entity
		 *a
		 * @param	entity the Entity where the Component was removed
		 * @param	sharedState the current user input state */
		virtual void onRemove(
			Entity /*entity*/, const ScriptSharedState& /*sharedState*/
		) {};

		/** Function called every clock tick
		 *
		 * @param	entity the Entity to update
		 * @param	sharedState the current user input state */
		virtual void onUpdate(
			Entity /*entity*/, const ScriptSharedState& /*sharedState*/
		) {};
	};


	/**
	 * Class ScriptComponent, it's used for changing the behaviour of an Entity
	 * at runtime
	 */
	class ScriptComponent
	{
	public:		// Nested types
		using ScriptResource = Repository::ResourceRef<Script>;

	private:	// Attributes
		/** The EventManager used for notifying the ScriptComponent changes */
		EventManager* mEventManager = nullptr;

		/** The Entity that owns the ScriptComponent */
		Entity mEntity = kNullEntity;

		/** The Script to execute */
		ScriptResource mScript;

		/** A pointer to the data that can be used for storing the per
		 * entity state of @see mScript */
		void* mScriptData = nullptr;

	public:		// Functions
		/** Sets the ScriptComponent attributes
		 *
		 * @param	eventManager the new EventManager of the ScriptComponent
		 * @param	entity the new Entity of the ScriptComponent */
		void setup(EventManager* eventManager, Entity entity);

		/** @return	the current Script of the ScriptComponent */
		const ScriptResource& getScript() const { return mScript; };

		/** Changes the Script of the ScriptComponent
		 *
		 * @param	script the new Script of the ScriptComponent */
		void setScript(const ScriptResource& script);

		/** @return	a pointer to the script data of the ScriptComponent */
		void* getScriptData() const { return mScriptData; };

		/** Sets the script data of the ScriptComponent
		 *
		 * @param	data a pointer to the new script data */
		void setScriptData(void* data) { mScriptData = data; };

		/** Function called when the ScriptComponent is added to an Entity
		 *
		 * @param	state the current state */
		void onAdd(const ScriptSharedState& state);

		/** Function called when the ScriptComponent is removed from an
		 * Entity
		 *
		 * @param	state the current state */
		void onRemove(const ScriptSharedState& state);

		/** Function called every clock tick
		 *
		 * @param	state the current state */
		void onUpdate(const ScriptSharedState& state);
	};

}

#endif		// SCRIPT_COMPONENT_H
