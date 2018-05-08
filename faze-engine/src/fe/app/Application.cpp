#include <chrono>
#include <thread>
#include <iostream>
#include "fe/utils/Logger.h"
#include "fe/window/WindowSystem.h"
#include "fe/graphics/GraphicsSystem.h"
#include "fe/physics/PhysicsEngine.h"
#include "fe/audio/AudioEngine.h"
#include "fe/app/Entity.h"
#include "fe/app/Application.h"
#include "fe/app/InputManager.h"
#include "fe/app/GraphicsManager.h"
#include "fe/app/PhysicsManager.h"
#include "fe/app/AudioManager.h"

namespace fe { namespace app {

	Application::Application(const std::string& title, int width, int height, float updateTime) :
		mUpdateTime(updateTime), mState(AppState::STOPPED)
	{
		try {
			// Window
			mWindowSystem = new window::WindowSystem({ title, width, height, false, false });
			std::cout << mWindowSystem->getGLInfo() << std::endl;

			// Input
			mInputManager = new InputManager(*mWindowSystem);

			// Graphics
			mGraphicsSystem = new graphics::GraphicsSystem();
			mGraphicsManager = new GraphicsManager(*mGraphicsSystem);

			// Physics
			mPhysicsEngine = new physics::PhysicsEngine();
			mPhysicsManager = new PhysicsManager(*mPhysicsEngine);

			// Audio
			mAudioEngine = new audio::AudioEngine();
			mAudioManager = new AudioManager(*mAudioEngine);
		}
		catch (std::exception& e) {
			mState = AppState::ERROR;
			utils::Logger::getInstance().write(
				utils::LogLevel::ERROR,
				"Error creating the application: " + std::string(e.what())
			);
		}
	}


	Application::~Application()
	{
		delete mAudioManager;
		delete mAudioEngine;
		delete mPhysicsManager;
		delete mPhysicsEngine;
		delete mGraphicsManager;
		delete mGraphicsSystem;
		delete mInputManager;
		delete mWindowSystem;
	}


	bool Application::run()
	{
		init();

		if (mState != AppState::STOPPED) {
			utils::Logger::getInstance().write(utils::LogLevel::ERROR, "Bad game state");
			return false;
		}

		/*********************************************************************
		 * MAIN LOOP
		 *********************************************************************/
		mState = AppState::RUNNING;
		float lastTime = mWindowSystem->getTime();
		while (mState == AppState::RUNNING) {
			// Calculate the elapsed time since the last update
			float curTime	= mWindowSystem->getTime();
			float deltaTime	= curTime - lastTime;

			if (deltaTime >= mUpdateTime) {
				lastTime = curTime;
				std::cout << deltaTime << "ms\r";

				// Update the Systems
				mWindowSystem->update();
				if (mWindowSystem->getInputData()->keys[GLFW_KEY_ESCAPE]) {
					mState = AppState::STOPPED;
				}
				mInputManager->update();
				mPhysicsManager->update(deltaTime);
				mAudioManager->update();
				mGraphicsManager->update();

				// Draw
				mGraphicsManager->render();
				mWindowSystem->swapBuffers();
			}
			else {
				std::this_thread::sleep_for( std::chrono::duration<float>(mUpdateTime - deltaTime) );
			}
		}

		end();

		return true;
	}

}}
