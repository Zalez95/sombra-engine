#ifndef INPUT_DATA_H
#define INPUT_DATA_H

#include <array>

namespace fe { namespace window {

	/**
	 * Struct InputData, it holds all the current data of the player input
	 */
	struct InputData
	{
		/** Max number of buttons in the mouse */
		static const int sMaxMouseButtons	= 32;

		/** Max number of buttons in the keys */
		static const int sMaxKeys			= 1024;

		/** An array with the keyboard pressed keys */
		std::array<bool, sMaxKeys> keys;

		/** An array with the mouse pressed keys */
		std::array<bool, sMaxMouseButtons> mouseButtons;

		/** The Mouse X coordinate relative to the top left corner of the
		 * window*/
		float mouseX;

		/** The Mouse Y coordinate relative to the top left corner of the
		 * window */
		float mouseY;

		/** Creates a new InputData */
		InputData() : mouseX(0.0f), mouseY(0.0f) {};

		/** Destructor */
		~InputData() {};
	};

}}

#endif		// INPUT_DATA_H
