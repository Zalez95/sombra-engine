#include <stdexcept>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "se/window/WindowSystem.h"
#include "se/utils/Log.h"

namespace se::window {

	WindowSystem::WindowSystem(const WindowData& windowData) : mWindowData(windowData), mWindow(nullptr)
	{
		// Init GLFW
		if (!glfwInit()) {
			throw std::runtime_error("Failed to initialize GLFW");
		}

		// Create the window
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


	void WindowSystem::setFullscreen(bool isFullscreen)
	{
		if (isFullscreen) {
			int numMonitors = 0;
			GLFWmonitor** monitors = glfwGetMonitors(&numMonitors);
			if (numMonitors > 0) {
				const GLFWvidmode* mode = glfwGetVideoMode(monitors[0]);
				glfwSetWindowMonitor(mWindow, monitors[0], 0, 0, mode->width, mode->height, mode->refreshRate);
			}
		}
		else {
			glfwSetWindowMonitor(mWindow, nullptr, 0, 0, mWindowData.width, mWindowData.height, GLFW_DONT_CARE);
		}
	}


	void WindowSystem::setResizable(bool isResizable)
	{
		glfwWindowHint(GLFW_RESIZABLE, isResizable);
	}


	void WindowSystem::setVsync(bool hasVsync)
	{
		glfwSwapInterval(hasVsync);
	}


	void WindowSystem::setCursorVisibility(bool isVisible)
	{
		glfwSetInputMode(mWindow, GLFW_CURSOR, isVisible? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
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
				auto userWindow = reinterpret_cast<WindowSystem*>(glfwGetWindowUserPointer(window));
				userWindow->onKeyCB(
					button,
					(action == GLFW_PRESS)? ButtonState::Pressed :
					(action == GLFW_REPEAT)? ButtonState::Repeated :
					ButtonState::Released
				);
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
				userWindow->onMouseButtonCB(
					button,
					(action == GLFW_PRESS)? ButtonState::Pressed :
					ButtonState::Released
				);
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
