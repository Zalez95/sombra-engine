#ifndef WINDOW_SYSTEM_H
#define WINDOW_SYSTEM_H

#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "InputData.h"

struct GLFWwindow;

namespace window {

	// GLFW Callbacks
    void error_callback(int error, const char* description);
    void key_callback(
		GLFWwindow* window, int key, int scancode, int action, int mods
	);
	void mouse_button_callback(
		GLFWwindow* window, int button, int action, int mods
	);
	void cursor_position_callback(
		GLFWwindow* window, double xpos, double ypos
	);


	/**
	 * WindowSystem Class
	 */
	class WindowSystem
	{
	private:	// Attributes
		/** The Window title */
		std::string mTitle;

		/** The Window width */
		int mWidth;

		/** The Window height */
		int mHeight;

		/** If the window is in fullscreen mode or not */
		bool mFullscreen;

		/** A pointer to the GLFW Window */
		GLFWwindow*	mWindow;

		/** The input data that we will update each frame */
		InputData mInputData;

	public:		// Functions
		/** Creates a new Window, the viewport and sets the callbacks for
		 * the events
		 *
		 * @param	title the title we want to show in the title bar of the
		 *			window
		 * @param	width the width of the window
		 * @param	height the height of the window
		 * @param	fullscreen true if the window must be in fullscreen mode,
		 *			false if it must be in windowed mode (by default) */
		WindowSystem(
			const std::string& title, int width, int height,
			bool fullscreen = false
		);
		
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
		inline int getWidth() const { return mWidth; };

		/** @return	the height of the window */
		inline int getHeight() const { return mHeight; };
		
		/** @return	true if the window is in fullsize mode */
		inline bool isFullscreen() const { return mFullscreen; };

		/** @return	true if the window is closed */
		bool isClosed() const;

		/** @return	the elapsed time we started the window */
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
