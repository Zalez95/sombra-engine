#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <memory>
#include <deque>
#include "ICommand.h"
#include "../window/WindowSystem.h"

namespace se::app {

	/**
	 * Class InputManager, it's a Manager used for handling the user input at a
	 * constant time interval
	 */
	class InputManager
	{
	private:	// Nested types
		using KeyType = int;
		using ICommandUPtr = std::unique_ptr<ICommand>;
		using MouseCommandUPtr = std::unique_ptr<MouseCommand>;
		using ScrollCommandUPtr = std::unique_ptr<ScrollCommand>;
		using ResizeCommandUPtr = std::unique_ptr<ResizeCommand>;

	private:	// Attributes
		/** Max number of buttons in the keyboard */
		static constexpr int kMaxKeys = 1024;

		/** Max number of buttons in the mouse */
		static constexpr int kMaxMouseButtons = 32;

		/** The number of state in which a button can be */
		static constexpr int kNStates =
			static_cast<int>(window::ButtonState::NumStates);

		/** A reference to the WindowSystem used for checking the player's
		 * input data */
		window::WindowSystem& mWindowSystem;

		/** Maps each key code with its respective Commands to execute after
		 * the user input */
		ICommandUPtr mKeyCommands[kMaxKeys][kNStates];

		/** Maps each mouse button code with its respective Commands to execute
		 * after the user input */
		ICommandUPtr mButtonCommands[kMaxMouseButtons][kNStates];

		/** The command to execute when the mouse location changes */
		MouseCommandUPtr mMouseCommand;

		/** The command to execute when the scroll state changes */
		ScrollCommandUPtr mScrollCommand;

		/** The command to execute when the window size changes */
		ResizeCommandUPtr mResizeCommand;

		/** A queue that holds all the commands that must be executed in the
		 * next update call due to the user input from oldest to newest */
		std::deque<ICommand*> mCommandQueue;

	public:		// Functions
		/** Creates a new InputManager
		 *
		 * @param	windowSystem the WindowSystem used for Checking the
		 *			input of the player */
		InputManager(window::WindowSystem& windowSystem);

		/** Adds the given key command to execute
		 *
		 * @param	keyCode the code of the key
		 * @param	state the state in which the key must be
		 * @param	command a pointer to the command to execute */
		void addKeyCommand(
			KeyType keyCode, window::ButtonState state,
			ICommandUPtr command
		);

		/** Adds the given mouse button command to execute
		 *
		 * @param	buttonCode the code of the mouse button
		 * @param	state the state in which the mouse button must be
		 * @param	command a pointer to the command to execute */
		void addButtonCommand(
			KeyType buttonCode, window::ButtonState state,
			ICommandUPtr command
		);

		/** Sets the mouse movement command to execute
		 *
		 * @param	command a pointer to the command to execute */
		void setMouseCommand(MouseCommandUPtr command);

		/** Sets the scroll movement command to execute
		 *
		 * @param	command a pointer to the command to execute */
		void setScrollCommand(ScrollCommandUPtr command);

		/** Sets the resize command to execute
		 *
		 * @param	command a pointer to the command to execute */
		void setResizeCommand(ResizeCommandUPtr command);

		/** Executes all the user input commands */
		void update();
	};

}

#endif		// INPUT_MANAGER_H
