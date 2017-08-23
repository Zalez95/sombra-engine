#include "InputManager.h"
#include <algorithm>
#include "Entity.h"

namespace game {

// Static variables definition
	const float InputManager::RUN_SPEED		= 100.0f;
	const float InputManager::JUMP_SPEED	= 50.0f;
	const float InputManager::MOUSE_SPEED	= 15.0f;

// Public functions definition
	void InputManager::addEntity(Entity* entity)
	{
		if (!entity) return;

		mEntities.push_back(entity);
	}


	void InputManager::removeEntity(Entity* entity)
	{
		mEntities.erase(
			std::remove(mEntities.begin(), mEntities.end(), entity),
			mEntities.end()
		);
	}


	void InputManager::update(float delta)
	{
		// Get the player's input data
		const window::InputData* inputData = mWindowSystem.getInputData();
		
		if (inputData) {
			// Update the entities
			for (Entity* entity : mEntities) {
				doMouseInput(entity, *inputData, delta);
				doKeyboardInput(entity, *inputData, delta);
			}

			resetMousePosition();
		}
	}

// Private functions definition
	void InputManager::doMouseInput(Entity* entity, const window::InputData& inputData, float delta) const
	{
		float width			= static_cast<float>(mWindowSystem.getWidth());
		float height		= static_cast<float>(mWindowSystem.getHeight());

		// Get the mouse movement from the center of the screen in the range [-1, 1]
		glm::vec2 mouseDelta(
			2 * inputData.mMouseX / width - 1.0f,
			2 * inputData.mMouseY / height - 1.0f		// note that the Y position is upsidedown
		);

		// Calculate the rotation around the Entity's y-axis
		float yaw			= (MOUSE_SPEED * delta) * mouseDelta.x;
		glm::quat qYaw		= glm::angleAxis(yaw, glm::vec3(0, 1, 0));

		// Calculate the rotation around the Entity's x-axis
		float pitch			= (MOUSE_SPEED * delta) * mouseDelta.y;
		glm::quat qPitch	= glm::angleAxis(pitch, glm::vec3(1, 0, 0));
		
		// Apply the change in orientation
		entity->mOrientation = glm::normalize((qPitch * qYaw) * entity->mOrientation);
	}


	void InputManager::doKeyboardInput(Entity* entity, const window::InputData& inputData, float delta) const
	{
		glm::vec3 forward	= glm::vec3(0, 0,-1) * entity->mOrientation;
		glm::vec3 up		= glm::vec3(0, 1, 0);
		glm::vec3 right		= glm::cross(forward, up);
		
		// Get the direction from the input in the XZ plane
		glm::vec3 direction;
		if (inputData.mKeys[GLFW_KEY_W]) { direction += forward; }
		if (inputData.mKeys[GLFW_KEY_S]) { direction -= forward; }
		if (inputData.mKeys[GLFW_KEY_D]) { direction += right; }
		if (inputData.mKeys[GLFW_KEY_A]) { direction -= right; }

		// Normalize the direction
		float len = glm::length(direction);
		if (len > 0) { direction /= len; }

		// Transform the direction to velocity
		entity->mPosition += RUN_SPEED * delta * direction;

		// Add the jump velocity
		if (inputData.mKeys[GLFW_KEY_SPACE]) { entity->mPosition += JUMP_SPEED * delta * up; }
		if (inputData.mKeys[GLFW_KEY_LEFT_CONTROL]) { entity->mPosition -= JUMP_SPEED * delta * up; }
	}


	void InputManager::resetMousePosition() const
	{
		float width		= static_cast<float>(mWindowSystem.getWidth());
		float height	= static_cast<float>(mWindowSystem.getHeight());
		mWindowSystem.setMousePosition(width / 2.0f, height / 2.0f);
	}

}
