#ifndef GAME_COMMANDS_H
#define GAME_COMMANDS_H

#include <array>
#include <se/window/WindowSystem.h>
#include <se/app/Application.h>
#include <se/app/ICommand.h>

namespace game {

	/** Holds the the transformations to apply to an Entity based on the
	 * user input */
	struct InputTransforms
	{
		enum class Direction : int
		{ Front = 0, Back, Right, Left, Up, Down, NumDirections };

		/** The rotation based on the mouse movement around the world Y axis
		 * @note	this values are framerate dependant */
		float yaw;

		/** The rotation based on the mouse movement around the Entity X axis
		 * @note	this values are framerate dependant */
		float pitch;

		/** The state of the movement in each direction */
		std::array<bool, static_cast<int>(Direction::NumDirections)> movement;

		/** Creates a new InputTransforms */
		InputTransforms() : yaw(0.0f), pitch(0.0f), movement{} {};
	};


	/**
	 * Class MouseFPSControl, it's a MouseCommand used to control an Entity
	 * orientation with the mouse like in a First Person Shooter
	 */
	class MouseFPSControl : public se::app::MouseCommand
	{
	private:	// Attributes
		/** A reference to the WindowSystem of the Game */
		se::window::WindowSystem& mWindowSystem;

		/** A reference to the InputTransforms that holds the pitch and yaw
		 * of an Entity */
		InputTransforms& mInputTransforms;

		/** The rotation speed of the Entity based on the mouse movement */
		float mMouseSpeed;

	public:		// Functions
		/** Creates a new MouseFPSControl
		 *
		 * @param	windowSystem a reference to the WindowSystem of the game
		 * @param	inputTransforms a reference to the InputTransforms that
		 *			holds the pitch an yaw of an Entity
		 * @param	mouseSpeed the rotation speed of the Entity based on the
		 *			mouse movement */
		MouseFPSControl(
			se::window::WindowSystem& windowSystem,
			InputTransforms& inputTransforms, float mouseSpeed
		);

		/** Executes the current command action */
		virtual void execute() override;
	private:
		/** Resets the mouse position to the center of the window */
		void resetMousePosition();
	};


	/**
	 * Class StartMoving, it's a ICommand used to control an Entity
	 * movement in one direction
	 */
	class StartMoving : public se::app::ICommand
	{
	private:	// Attributes
		/** The direction towards the entity should start moving */
		InputTransforms::Direction mDirection;

		/** A reference to the InputTransforms that holds the movement directions
		 * of an Entity */
		InputTransforms& mInputTransforms;

	public:		// Functions
		/** Creates a new StartMoving
		 *
		 * @param	direction the direction towards the entity should start
		 *			moving
		 * @param	inputTransforms a reference to the InputTransforms that
		 *			holds the movement directions of an Entity */
		StartMoving(
			InputTransforms::Direction direction, InputTransforms& inputTransforms
		) : mDirection(direction), mInputTransforms(inputTransforms) {};

		/** Executes the current command action */
		virtual void execute() override;
	};


	/**
	 * Class StopMoving, it's a ICommand used to control an Entity
	 * movement in one direction
	 */
	class StopMoving : public se::app::ICommand
	{
	private:	// Attributes
		/** The direction towards the entity should stop moving */
		InputTransforms::Direction mDirection;

		/** A reference to the InputTransforms that holds the movement directions
		 * of an Entity */
		InputTransforms& mInputTransforms;

	public:		// Functions
		/** Creates a new StopMoving
		 *
		 * @param	direction the direction towards the entity should stop
		 *			moving
		 * @param	inputTransforms a reference to the InputTransforms that
		 *			holds the movement directions of an Entity */
		StopMoving(
			InputTransforms::Direction direction, InputTransforms& inputTransforms
		) : mDirection(direction), mInputTransforms(inputTransforms) {};

		/** Executes the current command action */
		virtual void execute() override;
	};


	/**
	 * Class CloseCommand, it's a ICommand used for stopping the Application
	 */
	class CloseCommand : public se::app::ICommand
	{
	private:	// Attributes
		/** A reference to the application to stop */
		se::app::Application& mApplication;

	public:		// Functions
		/** Creates a new CloseCommand
		 *
		 * @param	application a reference to the application to stop */
		CloseCommand(se::app::Application& application) :
			mApplication(application) {};

		/** Executes the current command action */
		virtual void execute() override;
	};

}

#endif		// GAME_COMMANDS_H
