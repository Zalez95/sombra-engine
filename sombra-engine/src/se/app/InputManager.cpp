#include <algorithm>
#include "se/utils/Log.h"
#include "se/app/InputManager.h"
#include "se/app/Entity.h"

namespace se::app {

	InputManager::InputManager(window::WindowSystem& windowSystem) : mWindowSystem(windowSystem)
	{
		mWindowSystem.onKey([&](int keyCode, window::ButtonState state) {
			SOMBRA_DEBUG_LOG << "Key " << keyCode << " (" << static_cast<char>(keyCode) << ") with state " << static_cast<int>(state);

			auto command = mKeyCommands[keyCode][static_cast<int>(state)].get();
			if (command) {
				mCommandQueue.push_back(command);
			}
		});

		mWindowSystem.onMouseButton([&](int buttonCode, window::ButtonState state) {
			SOMBRA_DEBUG_LOG << "Mouse button " << buttonCode << " with state " << static_cast<int>(state);

			auto command = mButtonCommands[buttonCode][static_cast<int>(state)].get();
			if (command) {
				mCommandQueue.push_back(command);
			}
		});

		mWindowSystem.onMouseMove([&](double x, double y) {
			auto command = mMouseCommand.get();
			if (command) {
				command->setMousePosition(x, y);
				if (std::find(mCommandQueue.begin(), mCommandQueue.end(), command) == mCommandQueue.end()) {
					mCommandQueue.push_back(command);
				}
			}
		});

		mWindowSystem.onScroll([&](double x, double y) {
			auto command = mScrollCommand.get();
			if (command) {
				command->setScrollPosition(x, y);
				if (std::find(mCommandQueue.begin(), mCommandQueue.end(), command) == mCommandQueue.end()) {
					mCommandQueue.push_back(command);
				}
			}
		});

		mWindowSystem.onResize([&](double x, double y) {
			auto command = mResizeCommand.get();
			if (command) {
				command->setWindowSize(x, y);
				if (std::find(mCommandQueue.begin(), mCommandQueue.end(), command) == mCommandQueue.end()) {
					mCommandQueue.push_back(command);
				}
			}
		});
	}


	void InputManager::addKeyCommand(KeyType keyCode, window::ButtonState state, ICommandUPtr command)
	{
		mKeyCommands[keyCode][static_cast<int>(state)] = std::move(command);
	}


	void InputManager::addButtonCommand(KeyType buttonCode, window::ButtonState state, ICommandUPtr command)
	{
		mButtonCommands[buttonCode][static_cast<int>(state)] = std::move(command);
	}


	void InputManager::setMouseCommand(MouseCommandUPtr command)
	{
		mMouseCommand = std::move(command);
	}


	void InputManager::setScrollCommand(ScrollCommandUPtr command)
	{
		mScrollCommand = std::move(command);
	}


	void InputManager::setResizeCommand(ResizeCommandUPtr command)
	{
		mResizeCommand = std::move(command);
	}


	void InputManager::update()
	{
		SOMBRA_INFO_LOG << "Updating the InputManager. CommandQueue size = " << mCommandQueue.size();

		while (!mCommandQueue.empty()) {
			ICommand* currentCommand = mCommandQueue.front();
			mCommandQueue.pop_front();

			currentCommand->execute();
		}

		SOMBRA_INFO_LOG << "InputManager updated";
	}

}
