#ifndef WINDOW_SYSTEM_H
#define WINDOW_SYSTEM_H

#include <string>
#include <functional>

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

		/** If the cursor must be shown or not */
		bool cursorVisibility;
	};


	/** The different states in which a mouse button or key can be */
	enum class ButtonState : int
	{
		Released = 0,
		Pressed,
		NumStates
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
		GLFWwindow* mWindow;

		/** The callback to execute when the window size changes */
		std::function<void(int, int)> onResizeCB;

		/** The callback to execute when the mouse moves */
		std::function<void(double, double)> onMouseMoveCB;

		/** The callback to execute when the scroll state changes */
		std::function<void(double, double)> onScrollCB;

		/** The callback to execute when a key state changes */
		std::function<void(int, ButtonState)> onKeyCB;

		/** The callback to execute when a mouse button state changes */
		std::function<void(int, ButtonState)> onMouseButtonCB;

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

		/** @return	true if the window is closed */
		bool isClosed() const;

		/** Sets the mouse position in the window
		 *
		 * @param	x the position of the mouse in the X axis
		 * @param	y the position of the mouse in the Y axis */
		void setMousePosition(double x, double y);

		/** Sets the callback function to execute when the window is resized
		 *
		 * @param	callback the function to execute. It must take as parameters
		 *			the new width and height of the window */
		void onResize(const std::function<void(int, int)>& callback);

		/** Sets the callback function to execute when the mouse moves
		 *
		 * @param	callback the function to execute. It must take as parameters
		 *			the x and y position of the mouse cursor */
		void onMouseMove(const std::function<void(double, double)>& callback);

		/** Sets the callback function to execute when the scroll changes
		 *
		 * @param	callback the function to execute. It must take as parameters
		 *			the x and y position of the scroll */
		void onScroll(const std::function<void(double, double)>& callback);

		/** Sets the callback function to execute when the key state changes
		 *
		 * @param	callback the function to execute. It must take as parameters
		 *			the key code and its state */
		void onKey(
			const std::function<void(int, ButtonState)>& callback
		);

		/** Sets the callback function to execute when the mouse button state
		 * changes
		 *
		 * @param	callback the function to execute. It must take as parameters
		 *			the mouse button code and its state */
		void onMouseButton(
			const std::function<void(int, ButtonState)>& callback
		);

		/** Retrieves the window events and Sets the pressed keys by the
		 * user to the Entity that the System must update */
		void update();

		/** Swaps the front and back buffers of the window.
		 * @note	The front buffer of the window is the one currently being
		 *			displayed and the back buffer contains the new rendered
		 *			frame */
		void swapBuffers();
	};

}

#endif		// WINDOW_SYSTEM_H
