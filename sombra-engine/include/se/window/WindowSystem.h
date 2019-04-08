#ifndef WINDOW_SYSTEM_H
#define WINDOW_SYSTEM_H

#include <string>
#include "InputData.h"

struct GLFWwindow;


namespace se::window {

	/**
	 * Struct Window Data, it holds the raw data used to create a window,
	 * like its flags and size
	 */
	struct WindowData
	{
		/** The window title */
		std::string title;

		/** The window width */
		int width;

		/** The window height */
		int height;

		/** If the window is in fullscreen mode or not */
		bool fullscreen;

		/** If the window is resizable or not */
		bool resizable;

		/** If the window has v-sync enabled or not */
		bool vsync;
	};


	/**
	 * Class WindowSystem, It's used to create and manage windows and checking
	 * the user input data (polling)
	 */
	class WindowSystem
	{
	private:	// Attributes
		/** The data of the window, like its flags and size */
		WindowData mWindowData;

		/** A pointer to the GLFW Window */
		GLFWwindow*	mWindow;

		/** The input data that we will update each frame */
		InputData mInputData;

	public:		// Functions
		/** Creates a new Window, the viewport and sets the callbacks for
		 * the events
		 *
		 * @param	windowData the window data used to create the Window
		 * @throw	runtime_error if the window couldn't be created */
		WindowSystem(const WindowData& windowData);

		/** Class destructor, destroys the window and stops GLFW */
		~WindowSystem();

		/** @return	the current window data of the WindowSystem */
		const WindowData& getWindowData() const { return mWindowData; };

		/** @return	the current data of the input inserted by the player,
		 *			like the pressed mouse buttons, keyboard keys and the
		 *			position of the mouse (Key polling) */
		const InputData& getInputData() const { return mInputData; };

		/** @return	true if the window is closed */
		bool isClosed() const;

		/** Retrieves the window events and Sets the pressed keys by the
		 * user to the Entity that the System must update */
		void update();

		/** Swaps the front and back buffers of the window.
		 * @note	The front buffer of the window is the one currently being
		 *			displayed and the back buffer contains the new rendered
		 *			frame */
		void swapBuffers();

		/** Sets the mouse position in the window
		 *
		 * @param	x the position of the mouse in the X axis
		 * @param	y the position of the mouse in the Y axis */
		void setMousePosition(double x, double y);

		/** Sets the cursor visibility
		 *
		 * @param	visible if the cursor is going to be visible or not */
		void setCursorVisibility(bool visible);
	};

}

#endif		// WINDOW_SYSTEM_H
