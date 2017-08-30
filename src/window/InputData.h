#ifndef INPUT_DATA_H
#define INPUT_DATA_H

// Input constants
#define MAX_MOUSE_BUTTONS	32
#define MAX_KEYS			1024

namespace window {

	/**
	 * Struct InputData, it holds all the current data of the player input
	 */
	struct InputData
	{
		/** An array with the keyboard pressed keys */
		bool mKeys[MAX_KEYS];

		/** An array with the mouse pressed keys */
		bool mMouseButtons[MAX_MOUSE_BUTTONS];

		/** The Mouse X coordinate relative to the top left corner of the
		 * window*/
		float mMouseX;

		/** The Mouse Y coordinate relative to the top left corner of the
		 * window */
		float mMouseY;

		/** Creates a new InputData */
		InputData() : mMouseX(0.0f), mMouseY(0.0f)
		{
			for (size_t i = 0; i < MAX_KEYS; ++i) {
				mKeys[i] = false;
			}

			for (size_t i = 0; i < MAX_MOUSE_BUTTONS; ++i) {
				mMouseButtons[i] = false;
			}
		};

		/** Destructor */
		~InputData() {};
	};

}

#endif		// INPUT_DATA_H
