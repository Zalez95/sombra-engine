#include "fe/window/WindowSystem.h"
#include <stdexcept>

namespace fe { namespace window {

// Callback functions
	void error_callback(int error, const char* description)
	{
		throw std::runtime_error("Window System: Error " + std::to_string(error) + ": " + description);
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
	WindowSystem::WindowSystem(const WindowData& windowData) :
		mWindowData(windowData), mWindow(nullptr), mInputData()
	{
		// 1. Init GLFW
		if (!glfwInit()) {
			throw std::runtime_error("Failed to initialize GLFW");
		}

		// 2. Create the window
		glfwWindowHint(GLFW_SAMPLES, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, mWindowData.mResizable);

		mWindow = glfwCreateWindow(mWindowData.mWidth, mWindowData.mHeight, mWindowData.mTitle.c_str(), nullptr, nullptr);
		if (!mWindow) {
			glfwTerminate();
			throw std::runtime_error("Failed to create the Window");
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
		glewExperimental = true;
		if (glewInit() != GLEW_OK) {
			glfwDestroyWindow(mWindow);
			glfwTerminate();
			throw std::runtime_error("Failed to initialize GLEW");
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

}}