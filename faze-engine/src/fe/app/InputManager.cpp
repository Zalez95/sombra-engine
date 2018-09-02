#include <algorithm>
#include "fe/app/InputManager.h"
#include "fe/app/Entity.h"

namespace fe { namespace app {

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


	void InputManager::update()
	{
		// Get the player's input data
		const window::InputData* inputData = mWindowSystem.getInputData();

		if (inputData) {
			// Update the entities
			for (Entity* entity : mEntities) {
				doMouseInput(entity, *inputData);
				doKeyboardInput(entity, *inputData);
			}

			resetMousePosition();
		}
	}

// Private functions definition
	void InputManager::doMouseInput(Entity* entity, const window::InputData& inputData) const
	{
		float width			= static_cast<float>(mWindowSystem.getWidth());
		float height		= static_cast<float>(mWindowSystem.getHeight());

		// Get the mouse movement from the center of the screen in the range [-1, 1]
		glm::vec2 mouseDelta(
			2 * inputData.mouseX / width - 1.0f,
			2 * inputData.mouseY / height - 1.0f		// note that the Y position is upsidedown
		);

		// Calculate the rotation around the Entity's y-axis
		float yaw			= kMouseSpeed * mouseDelta.x;
		glm::quat qYaw		= glm::angleAxis(yaw, glm::vec3(0, 1, 0));

		// Calculate the rotation around the Entity's x-axis
		float pitch			= kMouseSpeed * mouseDelta.y;
		glm::quat qPitch	= glm::angleAxis(pitch, glm::vec3(1, 0, 0));

		// Apply the change in orientation
		entity->orientation = glm::normalize((qPitch * qYaw) * entity->orientation);
	}


	void InputManager::doKeyboardInput(Entity* entity, const window::InputData& inputData) const
	{
		glm::vec3 forward	= glm::vec3(0, 0,-1) * entity->orientation;
		glm::vec3 up		= glm::vec3(0, 1, 0);
		glm::vec3 right		= glm::cross(forward, up);

		// Get the direction from the input in the XZ plane
		glm::vec3 direction(0.0f);
		if (inputData.keys[GLFW_KEY_W]) { direction += forward; }
		if (inputData.keys[GLFW_KEY_S]) { direction -= forward; }
		if (inputData.keys[GLFW_KEY_D]) { direction += right; }
		if (inputData.keys[GLFW_KEY_A]) { direction -= right; }

		// Normalize the direction
		float length = glm::length(direction);
		if (length > 0) { direction /= length; }

		// Transform the direction to velocity
		float velocityDiff = kRunSpeed - glm::length(entity->velocity);
		if (velocityDiff > 0) {
			entity->velocity += velocityDiff * direction;
		}

		// Add the jump velocity
		if (inputData.keys[GLFW_KEY_SPACE]) { entity->velocity += kJumpSpeed * up; }
		if (inputData.keys[GLFW_KEY_LEFT_CONTROL]) { entity->velocity -= kJumpSpeed * up; }
	}


	void InputManager::resetMousePosition() const
	{
		float width		= static_cast<float>(mWindowSystem.getWidth());
		float height	= static_cast<float>(mWindowSystem.getHeight());
		mWindowSystem.setMousePosition(width / 2.0f, height / 2.0f);
	}

}}
