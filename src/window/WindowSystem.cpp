#include "WindowSystem.h"
#include "../utils/Logger.h"

namespace window {
	
// Callback functions
	void error_callback(int error, const char* description)
	{
		utils::Logger::writeLog(utils::LogType::ERROR, "Window System: Error " + std::to_string(error) + ": " + description);
	}


	void key_callback(GLFWwindow* window, int button, int /*scancode*/, int action, int /*mods*/)
	{
		auto userWindow = reinterpret_cast<WindowSystem*>(glfwGetWindowUserPointer(window));

		if (action != GLFW_RELEASE) {
			userWindow->mInputData.mKeys[button] = true;
		}
		else {
			userWindow->mInputData.mKeys[button] = false;
		}
	}


	void mouse_button_callback(GLFWwindow* window, int button, int action, int /*mods*/)
	{
		auto userWindow = reinterpret_cast<WindowSystem*>(glfwGetWindowUserPointer(window));

		if (action != GLFW_RELEASE) {
			userWindow->mInputData.mMouseButtons[button] = true;
		}
		else {
			userWindow->mInputData.mMouseButtons[button] = false;
		}
	}


	void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
	{
		auto userWindow = reinterpret_cast<WindowSystem*>(glfwGetWindowUserPointer(window));

		userWindow->mInputData.mMouseX = float(xpos);
		userWindow->mInputData.mMouseY = float(ypos);
	}

// Public functions
	WindowSystem::WindowSystem(const std::string& title, int width, int height, bool fullscreen) :
		mTitle(title), mWidth(width), mHeight(height), mFullscreen(fullscreen), mWindow(nullptr), mInputData()
	{
		// 1. Init GLFW
		if (!glfwInit()) {
			utils::Logger::writeLog(utils::LogType::ERROR, "Failed to initialize GLFW");
			return;
		}

		// 2. Create the window
		mWindow = glfwCreateWindow( mWidth, mHeight, mTitle.c_str(), nullptr, nullptr );
		if (!mWindow) {
			utils::Logger::writeLog(utils::LogType::ERROR, "Failed to create the Window");
			glfwTerminate();
			return;
		}

		glfwMakeContextCurrent(mWindow);
		glfwSetWindowUserPointer(mWindow, this);
		glfwSwapInterval(0);

		// 3. Set Input event callbacks
		glfwSetErrorCallback(error_callback);
		glfwSetKeyCallback(mWindow, key_callback);
		glfwSetMouseButtonCallback(mWindow, mouse_button_callback);
		glfwSetCursorPosCallback(mWindow, cursor_position_callback);

		// 4. Init GLEW
		if (glewInit() != GLEW_OK) {
			utils::Logger::writeLog(utils::LogType::ERROR, "Failed to initialize GLEW");
			glfwDestroyWindow(mWindow);
			glfwTerminate();
			return;
		}

		// 5. Set the viewport
		setViewport();
	}


	WindowSystem::~WindowSystem()
	{
		glfwDestroyWindow(mWindow);
		glfwTerminate();
	}


	void WindowSystem::update()
	{
		for (size_t i = 0; i < MAX_KEYS; ++i) {
			if (mInputData.mKeys[i] == true) {
				mInputData.mKeys[i] = false;
			}
		}

		for (size_t i = 0; i < MAX_MOUSE_BUTTONS; ++i) {
			if (mInputData.mMouseButtons[i] == true) { mInputData.mMouseButtons[i] = false; }
		}

		glfwPollEvents();
	}


	void WindowSystem::setMousePosition(float x, float y)
	{
		mInputData.mMouseX = x;
		mInputData.mMouseY = y;
		glfwSetCursorPos(mWindow, x, y);
	}


	bool WindowSystem::isClosed() const
	{
		return (glfwWindowShouldClose(mWindow) == 1);
	}


	float WindowSystem::getTime() const
	{
		return float(glfwGetTime());
	}


	void WindowSystem::swapBuffers()
	{
		glfwSwapBuffers(mWindow);
	}

	
	std::string WindowSystem::getGLInfo() const
	{
		return	"OpenGL Renderer: " + std::string(reinterpret_cast<const char*>(glGetString(GL_RENDERER))) +
				"\nOpenGL version supported " + std::string(reinterpret_cast<const char*>(glGetString(GL_VERSION))) +
				"\nGLSL version supported " + std::string(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
	}

// Private functions
	void WindowSystem::setViewport()
	{
		int width, height;
		glfwGetFramebufferSize(mWindow, &width, &height);
		glViewport(0, 0, width, height);
	}

}
