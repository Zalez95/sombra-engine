#include <chrono>
#include <thread>
#include <iostream>
#include "se/utils/Logger.h"
#include "se/window/WindowSystem.h"
#include "se/graphics/GraphicsSystem.h"
#include "se/physics/PhysicsEngine.h"
#include "se/audio/AudioEngine.h"
#include "se/app/Entity.h"
#include "se/app/Application.h"
#include "se/app/InputManager.h"
#include "se/app/GraphicsManager.h"
#include "se/app/PhysicsManager.h"
#include "se/app/CollisionManager.h"
#include "se/app/AudioManager.h"

namespace se::app {

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

			// Collision
			mCollisionDetector = new collision::CollisionDetector();
			mCollisionManager = new CollisionManager(*mCollisionDetector, *mPhysicsEngine);

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
		delete mCollisionManager;
		delete mCollisionDetector;
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
				mPhysicsManager->doDynamics(deltaTime);
				mCollisionManager->update(deltaTime);
				mPhysicsManager->doConstraints(deltaTime);
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

}
