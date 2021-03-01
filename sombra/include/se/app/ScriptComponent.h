#ifndef SCRIPT_COMPONENT_H
#define SCRIPT_COMPONENT_H

#include "EntityDatabase.h"
#include "events/EventManager.h"

namespace se::app {

	/**
	 * Struct InputData, it holds all the current user input
	 */
	struct UserInput
	{
		/** Max number of buttons in the mouse */
		static constexpr int kMaxMouseButtons = 8;

		/** Max number of buttons in the keys */
		static constexpr int kMaxKeys = 512;

		/** An array with the keyboard pressed keys */
		std::array<bool, kMaxKeys> keys = {};

		/** An array with the mouse pressed keys */
		std::array<bool, kMaxMouseButtons> mouseButtons = {};

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
	};


	/**
	 * Class ScriptComponent, it's used for changing the behaviour of an Entity
	 * at runtime
	 */
	class ScriptComponent
	{
	protected:
		/** The EntityDatabasethat holds @see mEntity */
		EntityDatabase* mEntityDatabase = nullptr;

		/** The EventManager used for creating events */
		EventManager* mEventManager = nullptr;

		/** The Entity that owns the ScriptComponent */
		Entity mEntity = kNullEntity;

	public:		// Functions
		/** Class destructor */
		virtual ~ScriptComponent() = default;

		/** Sets the ScriptComponent attributes
		 *
		 * @param	entityDatabase the new EntityDatabase of the ScriptComponent
		 * @param	entity the new Entity of the ScriptComponent
		 * @param	eventManager the new EventManager of the ScriptComponent */
		void setup(
			EntityDatabase& entityDatabase, EventManager& eventManager,
			Entity entity
		) {
			mEntityDatabase = &entityDatabase;
			mEventManager = &eventManager;
			mEntity = entity;
		};

		/** Function called when the ScriptComponent is added to an Entity
		 *
		 * @param	userInput the current user input state */
		virtual void onCreate(const UserInput& /*userInput*/) {};

		/** Function called when the ScriptComponent is removed from an
		 * Entity
		 *
		 * @param	userInput the current user input state */
		virtual void onDestroy(const UserInput& /*userInput*/) {};

		/** Function called every clock tick
		 *
		 * @param	elapsedTime the elapsed time in seconds since the last
		 *			update
		 * @param	userInput the current user input state */
		virtual void onUpdate(
			float /*elapsedTime*/, const UserInput& /*userInput*/
		) {};
	};

}

#endif		// SCRIPT_COMPONENT_H
