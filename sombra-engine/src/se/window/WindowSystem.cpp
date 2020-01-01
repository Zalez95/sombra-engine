#include <stdexcept>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "se/window/WindowSystem.h"
#include "se/utils/Log.h"

namespace se::window {

	WindowSystem::WindowSystem(const WindowData& windowData) :
		mWindowData(windowData), mWindow(nullptr)
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

		mWindow = glfwCreateWindow(mWindowData.width, mWindowData.height, mWindowData.title.c_str(), nullptr, nullptr);
		if (!mWindow) {
			glfwTerminate();
			throw std::runtime_error("Failed to create the Window");
		}

		glfwMakeContextCurrent(mWindow);
		glfwSetWindowUserPointer(mWindow, this);

		// 3. Config the window
		glfwSwapInterval(mWindowData.vsync? 1 : 0);
		glfwWindowHint(GLFW_RESIZABLE, mWindowData.resizable);
		glfwSetInputMode(mWindow, GLFW_CURSOR, mWindowData.cursorVisibility? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);

		glfwSetErrorCallback([](int error, const char* description)
		{
			SOMBRA_ERROR_LOG << "Error \"" << error << "\": " << description;
		});
		glfwSetWindowSizeCallback(mWindow, [](GLFWwindow* window, int width, int height)
		{
			auto userWindow = reinterpret_cast<WindowSystem*>(glfwGetWindowUserPointer(window));
			userWindow->mWindowData.width = width;
			userWindow->mWindowData.height = height;
		});
	}


	WindowSystem::~WindowSystem()
	{
		glfwDestroyWindow(mWindow);
		glfwTerminate();
	}


	bool WindowSystem::isClosed() const
	{
		return (glfwWindowShouldClose(mWindow) == 1);
	}


	void WindowSystem::setMousePosition(double x, double y)
	{
		glfwSetCursorPos(mWindow, x, y);
	}


	void WindowSystem::onResize(const std::function<void(int, int)>& callback)
	{
		if (callback) {
			onResizeCB = callback;

			glfwSetWindowSizeCallback(mWindow, [](GLFWwindow* window, int width, int height)
			{
				auto userWindow = reinterpret_cast<WindowSystem*>(glfwGetWindowUserPointer(window));
				userWindow->mWindowData.width = width;
				userWindow->mWindowData.height = height;

				userWindow->onResizeCB(width, height);
			});
		}
	}


	void WindowSystem::onMouseMove(const std::function<void(double, double)>& callback)
	{
		if (callback) {
			onMouseMoveCB = callback;

			glfwSetCursorPosCallback(mWindow, [](GLFWwindow* window, double xpos, double ypos)
			{
				auto userWindow = reinterpret_cast<WindowSystem*>(glfwGetWindowUserPointer(window));
				userWindow->onMouseMoveCB(xpos, ypos);
			});
		}
	}


	void WindowSystem::onScroll(const std::function<void(double, double)>& callback)
	{
		if (callback) {
			onScrollCB = callback;

			glfwSetScrollCallback(mWindow, [](GLFWwindow* window, double xoffset, double yoffset)
			{
				auto userWindow = reinterpret_cast<WindowSystem*>(glfwGetWindowUserPointer(window));
				userWindow->onScrollCB(xoffset, yoffset);
			});
		}
	}


	void WindowSystem::onKey(const std::function<void(int, ButtonState)>& callback)
	{
		if (callback) {
			onKeyCB = callback;

			glfwSetKeyCallback(mWindow, [](GLFWwindow* window, int button, int /*scancode*/, int action, int /*mods*/)
			{
				if (action != GLFW_REPEAT) {
					auto userWindow = reinterpret_cast<WindowSystem*>(glfwGetWindowUserPointer(window));
					userWindow->onKeyCB(button, (action == GLFW_RELEASE)? ButtonState::Released : ButtonState::Pressed);
				}
			});
		}
	}


	void WindowSystem::onMouseButton(const std::function<void(int, ButtonState)>& callback)
	{
		if (callback) {
			onMouseButtonCB = callback;

			glfwSetMouseButtonCallback(mWindow, [](GLFWwindow* window, int button, int action, int /*mods*/)
			{
				auto userWindow = reinterpret_cast<WindowSystem*>(glfwGetWindowUserPointer(window));
				userWindow->onMouseButtonCB(button, (action == GLFW_RELEASE)? ButtonState::Released : ButtonState::Pressed);
			});
		}
	}


	void WindowSystem::update()
	{
		glfwPollEvents();
	}


	void WindowSystem::swapBuffers()
	{
		glfwSwapBuffers(mWindow);
	}

}
