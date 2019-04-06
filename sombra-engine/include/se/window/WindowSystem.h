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

		/** Retrieves the window events and Sets the pressed keys by the
		 * user to the Entity that the System must update */
		void update();

		/** @return	the current data of the input inserted by the player,
		 *			like the pressed mouse buttons, keyboard keys and the
		 *			position of the mouse (Key polling) */
		inline const InputData* getInputData() const { return &mInputData; };

		/** Sets the mouse position in the window
		 *
		 * @param	x the position of the mouse in the X axis
		 * @param	y the position of the mouse in the Y axis */
		void setMousePosition(float x, float y);

		/** @return	the width of the window */
		inline int getWidth() const { return mWindowData.width; };

		/** @return	the height of the window */
		inline int getHeight() const { return mWindowData.height; };

		/** @return	true if the window is closed */
		bool isClosed() const;

		/** @return	the elapsed time since we started the window in seconds */
		float getTime() const;

		/** Swaps the front and back buffers of the window.
		 * <br>The front buffer of the window is the one currently being
		 * displayed and the back buffer contains the new rendered frame */
		void swapBuffers();

		/** @return	the OpenGL version info */
		std::string getGLInfo() const;
	private:
		/** Creates a viewport with the same size of the window */
		void setViewport();

		// GLFW Callbacks
		friend void error_callback(int error, const char* description);
		friend void key_callback(
			GLFWwindow* window, int key, int scancode, int action, int mods
		);
		friend void mouse_button_callback(
			GLFWwindow* window, int button, int action, int mods
		);
		friend void cursor_position_callback(
			GLFWwindow* window, double xpos, double ypos
		);
	};

}

#endif		// WINDOW_SYSTEM_H
