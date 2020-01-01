#include <se/utils/Log.h>
#include <se/window/WindowSystem.h>
#include "GameCommands.h"

namespace game {

	MouseFPSControl::MouseFPSControl(
		se::window::WindowSystem& windowSystem, InputTransforms& inputTransforms, float mouseSpeed
	) : mWindowSystem(windowSystem), mInputTransforms(inputTransforms), mMouseSpeed(mouseSpeed)
	{
		resetMousePosition();
	}


	void MouseFPSControl::execute()
	{
		// Get the mouse movement from the center of the screen in the range [-1, 1]
		const se::window::WindowData& data = mWindowSystem.getWindowData();
		double mouseDeltaX = 2.0 * mX / data.width - 1.0;
		double mouseDeltaY = 1.0 - 2.0 * mY / data.height;	// note that the Y position is upsidedown

		// Multiply the values by the mouse speed
		mInputTransforms.yaw = mMouseSpeed * static_cast<float>(mouseDeltaX);
		mInputTransforms.pitch = mMouseSpeed * static_cast<float>(mouseDeltaY);

		resetMousePosition();
	}


	void MouseFPSControl::resetMousePosition()
	{
		SOMBRA_DEBUG_LOG << "Changing the mouse position to the center of the window";

		const se::window::WindowData& data = mWindowSystem.getWindowData();
		mWindowSystem.setMousePosition(data.width / 2.0, data.height / 2.0);
	}


	void StartMoving::execute()
	{
		mInputTransforms.movement[static_cast<int>(mDirection)] = true;
	}


	void StopMoving::execute()
	{
		mInputTransforms.movement[static_cast<int>(mDirection)] = false;
	}


	void CloseCommand::execute()
	{
		mApplication.stop();
	}

}
