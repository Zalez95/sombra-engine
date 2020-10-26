#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

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
	};


	/** The different states in which a mouse button or key can be */
	enum class ButtonState : int
	{
		Released = 0,
		Repeated,
		Pressed,
		NumStates
	};


	/**
	 * Class WindowManager, It's used to create and manage windows and checking
	 * the user input data (polling)
	 */
	class WindowManager
	{
	private:	// Attributes
		/** The config of the window */
		WindowData mWindowData;

		/** A pointer to the GLFW Window */
		GLFWwindow* mWindow;

		/** The callback to execute when the window close button is pressed */
		std::function<void()> onCloseCB;

		/** The callback to execute when the window minimize button is
		 * pressed */
		std::function<void(bool)> onMinimizeCB;

		/** The callback to execute when the window size changes */
		std::function<void(int, int)> onResizeCB;

		/** The callback to execute when the mouse moves */
		std::function<void(double, double)> onMouseMoveCB;

		/** The callback to execute when the scroll state changes */
		std::function<void(double, double)> onScrollCB;

		/** The callback to execute when a key state changes */
		std::function<void(int, ButtonState)> onKeyCB;

		/** The callback to execute when a new code point of an input text is
		 * added (keyboard layout dependent) */
		std::function<void(unsigned int)> onTextInputCB;

		/** The callback to execute when a mouse button state changes */
		std::function<void(int, ButtonState)> onMouseButtonCB;

	public:		// Functions
		/** Creates a new Window, the viewport and sets the callbacks for
		 * the events
		 *
		 * @param	windowData the window data used to create the Window
		 * @throw	runtime_error if the window couldn't be created */
		WindowManager(const WindowData& windowData);

		/** Class destructor, destroys the window and stops GLFW */
		~WindowManager();

		/** @return	the current window data of the WindowManager */
		const WindowData& getWindowData() const { return mWindowData; };

		/** @return	true if the window is closed */
		bool isClosed() const;

		/** Sets the window size
		 *
		 * @param	width the new window width
		 * @param	height the new window height */
		void setSize(int width, int height);

		/** Sets the window fullscreen mode on/off
		 *
		 * @param	isFullscreen if the window is in fullscreen mode or not */
		void setFullscreen(bool isFullscreen);

		/** Sets the window resize on/off
		 *
		 * @param	isResizable if the window is resizable or not */
		void setResizable(bool isResizable);

		/** Sets the VSync on/off
		 *
		 * @param	hasVsync if the window has VSync enabled or not */
		void setVSync(bool hasVsync);

		/** Sets the mouse position in the window
		 *
		 * @param	x the position of the mouse in the X axis
		 * @param	y the position of the mouse in the Y axis */
		void setMousePosition(double x, double y);

		/** Sets the cursor visibility on/off
		 *
		 * @param	isVisible if the cursor must be shown or not */
		void setCursorVisibility(bool isVisible);

		/** Sets the callback function to execute when the window close
		 * button is pressed
		 *
		 * @param	callback the function to execute */
		void onClose(const std::function<void()>& callback);

		/** Sets the callback function to execute when the mouse button state
		 * changes
		 *
		 * @param	callback the function to execute. It must take as parameter
		 *			true if the window was minimized or false if restored */
		void onMinimize(const std::function<void(bool)>& callback);

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
		 *			the x and y offsets of the scroll */
		void onScroll(const std::function<void(double, double)>& callback);

		/** Sets the callback function to execute when the key state changes
		 *
		 * @param	callback the function to execute. It must take as parameters
		 *			the key code and its state */
		void onKey(const std::function<void(int, ButtonState)>& callback);

		/** Sets the callback function to execute when a new code point of an
		 * input text is added (keyboard layout dependent)
		 *
		 * @param	callback the function to execute. It must take as parameter
		 *			the code point added */
		void onTextInput(const std::function<void(unsigned int)>& callback);

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

#endif		// WINDOW_MANAGER_H
