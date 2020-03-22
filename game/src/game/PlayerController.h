#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include <se/app/Entity.h>
#include <se/window/WindowSystem.h>
#include <se/app/CollisionManager.h>
#include <se/app/events/EventManager.h>
#include <se/app/events/KeyEvent.h>
#include <se/app/events/MouseEvent.h>
#include <se/graphics/3D/Renderable3D.h>
#include <se/graphics/2D/RenderableText.h>

namespace game {

	struct GameData;


	/**
	 * Class PlayerController, TODO:
	 */
	class PlayerController : public se::app::IEventListener
	{
	private:	// Nested Types
		enum class Direction : int
		{ Front = 0, Back, Right, Left, Up, Down, NumDirections };

	private:	// Attributes
		GameData& mGameData;
		se::app::Entity& mEntity;
		se::graphics::RenderableText& mPickText;

		static constexpr float kRunSpeed	= 2.5f;
		static constexpr float kJumpSpeed	= 3.0f;
		static constexpr float kMouseSpeed	= 100.0f;
		static constexpr float kPitchLimit	= 0.05f;

		/** The rotation based on the mouse movement around the world Y axis
		 * @note	this values are framerate dependant */
		float mYaw;

		/** The rotation based on the mouse movement around the Entity X axis
		 * @note	this values are framerate dependant */
		float mPitch;

		/** The state of the movement in each direction */
		std::array<bool, static_cast<int>(Direction::NumDirections)> mMovement;

		/** If the mouse left button was pressed or not */
		bool mClicked;

		std::shared_ptr<se::graphics::Mesh> mTetrahedronMesh;
		std::shared_ptr<se::graphics::Material> mYellowMaterial;

	public:		// Functions
		/** Creates a new PlayerController
		 *
		 * @param	gameData a reference to the GameData
		 * @param	entity a reference to the Entity to control as a player
		 * @param	pickText the text to write to */
		PlayerController(
			GameData& gameData,
			se::app::Entity& entity,
			se::graphics::RenderableText& pickText
		);

		/** Class destructor */
		~PlayerController();

		/** Used for updating the PlayerController at the same time than the
		 * Game does it
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds */
		void update(float deltaTime);

		/** Resets the mouse position to the center of the window */
		void resetMousePosition();

		/** Notifies the PlayerController of the given event
		 *
		 * @param	event the IEvent to notify */
		virtual void notify(const se::app::IEvent& event) override;
	private:
		/** Handles the given event
		 *
		 * @param	event the KeyEvent to handle */
		void onKeyEvent(const se::app::KeyEvent& event);

		/** Handles the given event
		 *
		 * @param	event the MouseEvent to handle */
		void onMouseEvent(const se::app::MouseEvent& event);
	};

}

#endif		// PLAYER_CONTROLLER_H
