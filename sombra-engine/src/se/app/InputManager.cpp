#include <algorithm>
#include "se/utils/Log.h"
#include "se/app/InputManager.h"
#include "se/app/Entity.h"

namespace se::app {

	InputManager::InputManager(window::WindowSystem& windowSystem) :
		mWindowSystem(windowSystem)
	{
		resetMousePosition();
	}


	void InputManager::addEntity(Entity* entity)
	{
		if (!entity) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added";
			return;
		}

		mEntities.push_back(entity);
		SOMBRA_INFO_LOG << "Entity " << entity << " added successfully";
	}


	void InputManager::removeEntity(Entity* entity)
	{
		auto itEntity = std::find(mEntities.begin(), mEntities.end(), entity);
		if (itEntity != mEntities.end()) {
			mEntities.erase(itEntity);
			SOMBRA_INFO_LOG << "Entity " << entity << " removed successfully";
		}
		else {
			SOMBRA_WARN_LOG << "Entity " << entity << " wasn't removed";
		}
	}


	void InputManager::update()
	{
		SOMBRA_INFO_LOG << "Updating the InputManager";

		// Update the entities
		const window::InputData& inputData = mWindowSystem.getInputData();
		for (Entity* entity : mEntities) {
			// Reset the Entity input update
			entity->updated.reset( static_cast<int>(Entity::Update::Input) );

			doMouseInput(entity, inputData);
			doKeyboardInput(entity, inputData);
		}

		resetMousePosition();

		SOMBRA_INFO_LOG << "InputManager updated";
	}

// Private functions definition
	void InputManager::doMouseInput(Entity* entity, const window::InputData& inputData) const
	{
		SOMBRA_DEBUG_LOG << "Calculating the new entity " << entity << " orientation";

		const window::WindowData& data = mWindowSystem.getWindowData();

		// Get the mouse movement from the center of the screen in the range [-1, 1]
		double mouseDeltaX = 2.0 * inputData.mouseX / data.width - 1.0;
		double mouseDeltaY = 2.0 * inputData.mouseY / data.height - 1.0;		// note that the Y position is upsidedown

		if ((mouseDeltaX != 0.0) && (mouseDeltaY != 0.0)) {
			// Calculate the rotation around the Entity's y-axis
			float yaw			= kMouseSpeed * static_cast<float>(mouseDeltaX);
			glm::quat qYaw		= glm::angleAxis(yaw, glm::vec3(0, 1, 0));

			// Calculate the rotation around the Entity's x-axis
			float pitch			= kMouseSpeed * static_cast<float>(mouseDeltaY);
			glm::quat qPitch	= glm::angleAxis(pitch, glm::vec3(1, 0, 0));

			// Apply the change in orientation
			entity->orientation = glm::normalize(qPitch * qYaw * entity->orientation);
			entity->updated.set( static_cast<int>(Entity::Update::Input) );
		}
	}


	void InputManager::doKeyboardInput(Entity* entity, const window::InputData& inputData) const
	{
		SOMBRA_DEBUG_LOG << "Calculating the new entity " << entity << " velocity";

		glm::vec3 forward	= glm::vec3(0, 0,-1) * entity->orientation;
		glm::vec3 up		= glm::vec3(0, 1, 0);
		glm::vec3 right		= glm::cross(forward, up);

		// Get the direction from the input in the XZ plane
		glm::vec3 direction(0.0f);
		if (inputData.keys[SE_KEY_W]) { direction += forward; }
		if (inputData.keys[SE_KEY_S]) { direction -= forward; }
		if (inputData.keys[SE_KEY_D]) { direction += right; }
		if (inputData.keys[SE_KEY_A]) { direction -= right; }

		// Add the movement velocity
		float length = glm::length(direction);
		if (length > 0.0f) {
			entity->velocity += kRunSpeed * direction / length;
			entity->updated.set( static_cast<int>(Entity::Update::Input) );
		}

		// Add the jump velocity
		if (inputData.keys[SE_KEY_SPACE]) {
			entity->velocity += kJumpSpeed * up;
			entity->updated.set( static_cast<int>(Entity::Update::Input) );
		}
		if (inputData.keys[SE_KEY_LEFT_CONTROL]) {
			entity->velocity -= kJumpSpeed * up;
			entity->updated.set( static_cast<int>(Entity::Update::Input) );
		}
	}


	void InputManager::resetMousePosition() const
	{
		SOMBRA_DEBUG_LOG << "Changing the mouse position to the center of the window";

		const window::WindowData& data = mWindowSystem.getWindowData();
		mWindowSystem.setMousePosition(data.width / 2.0, data.height / 2.0);
	}

}
