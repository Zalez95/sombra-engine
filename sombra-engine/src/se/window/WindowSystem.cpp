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

		glfwSetErrorCallback([](int error, const char* description)
		{
			SOMBRA_ERROR_LOG << "Error code: " << error << ", " << description;
		});
		glfwSetWindowSizeCallback(mWindow, [](GLFWwindow* window, int width, int height)
		{
			auto userWindow = reinterpret_cast<WindowSystem*>(glfwGetWindowUserPointer(window));
			userWindow->mWindowData.width = width;
			userWindow->mWindowData.height = height;
		});
		glfwSetKeyCallback(mWindow, [](GLFWwindow* window, int button, int /*scancode*/, int action, int /*mods*/)
		{
			auto userWindow = reinterpret_cast<WindowSystem*>(glfwGetWindowUserPointer(window));
			// note that GLFW key codes and SE key codes are the same
			if ((button >= 0) && (button < InputData::kMaxKeys)) {
				userWindow->mInputData.keys[button] = (action != GLFW_RELEASE);
			}
		});
		glfwSetMouseButtonCallback(mWindow, [](GLFWwindow* window, int button, int action, int /*mods*/)
		{
			auto userWindow = reinterpret_cast<WindowSystem*>(glfwGetWindowUserPointer(window));
			// note that GLFW mouse button codes and SE mouse button codes are the same
			if ((button >= 0) && (button < InputData::kMaxMouseButtons)) {
				userWindow->mInputData.mouseButtons[button] = (action != GLFW_RELEASE);
			}
		});
		glfwSetCursorPosCallback(mWindow, [](GLFWwindow* window, double xpos, double ypos)
		{
			auto userWindow = reinterpret_cast<WindowSystem*>(glfwGetWindowUserPointer(window));
			userWindow->mInputData.mouseX = xpos;
			userWindow->mInputData.mouseY = ypos;
		});
		glfwSetScrollCallback(mWindow, [](GLFWwindow* window, double xoffset, double yoffset)
		{
			auto userWindow = reinterpret_cast<WindowSystem*>(glfwGetWindowUserPointer(window));
			userWindow->mInputData.scrollX = xoffset;
			userWindow->mInputData.scrollY = yoffset;
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


	void WindowSystem::update()
	{
		for (bool& key : mInputData.keys) {
			key = false;
		}

		for (bool& button : mInputData.keys) {
			button = false;
		}

		glfwPollEvents();
	}


	void WindowSystem::swapBuffers()
	{
		glfwSwapBuffers(mWindow);
	}


	void WindowSystem::setMousePosition(double x, double y)
	{
		mInputData.mouseX = x;
		mInputData.mouseY = y;
		glfwSetCursorPos(mWindow, x, y);
	}


	void WindowSystem::setCursorVisibility(bool visible)
	{
		glfwSetInputMode(mWindow, GLFW_CURSOR, visible? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
	}

}
