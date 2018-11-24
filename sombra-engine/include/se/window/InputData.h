#ifndef INPUT_DATA_H
#define INPUT_DATA_H

#include <array>

namespace se::window {

	/**
	 * Struct InputData, it holds all the current data of the player input
	 */
	struct InputData
	{
		/** Max number of buttons in the mouse */
		static constexpr int kMaxMouseButtons = 32;

		/** Max number of buttons in the keys */
		static constexpr int kMaxKeys = 1024;

		/** An array with the keyboard pressed keys */
		std::array<bool, kMaxKeys> keys;

		/** An array with the mouse pressed keys */
		std::array<bool, kMaxMouseButtons> mouseButtons;

		/** The Mouse X coordinate relative to the top left corner of the
		 * window*/
		float mouseX;

		/** The Mouse Y coordinate relative to the top left corner of the
		 * window */
		float mouseY;

		/** Creates a new InputData */
		InputData() : mouseX(0.0f), mouseY(0.0f) {};
	};

}

#endif		// INPUT_DATA_H
