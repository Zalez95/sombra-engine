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

		/** The Mouse X coordinate */
		float mMouseX;

		/** The Mouse Y coordinate */
		float mMouseY;

		/** Creates a new InputData */
		InputData()
		{
			for (unsigned int i = 0; i < MAX_KEYS; ++i) {
				mKeys[i] = false;
			}

			for (unsigned int i = 0; i < MAX_MOUSE_BUTTONS; ++i) {
				mMouseButtons[i] = false;
			}
		};

		/** Destructor */
		~InputData() {};
	};

}

#endif		// INPUT_DATA_H
