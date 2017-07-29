#include "InputComponent.h"
#include "Entity.h"

namespace game {

// Static variables definition
	const float InputComponent::RUN_SPEED	= 100.0f;
	const float InputComponent::JUMP_SPEED	= 50.0f;
	const float InputComponent::MOUSE_SPEED	= 5.0f;

// Public functions definition
	void InputComponent::update(Entity& entity, float delta)
	{
		const window::InputData* inputData = mWindowSystem.getInputData();
		if (inputData) {
			doMouseInput(entity, *inputData, delta);
			doKeyboardInput(entity, *inputData, delta);
		}
	}

// Private functions definition
	void InputComponent::doMouseInput(
		Entity& entity,
		const window::InputData& inputData, float delta
	) const
	{
		if (delta == 0) return;

		float width			= float(mWindowSystem.getWidth());
		float height		= float(mWindowSystem.getHeight());

		// Get the mouse movement from the center of the screen
		glm::vec2 mouseDelta(inputData.mMouseX / width - 0.5f, inputData.mMouseY / height - 0.5f);
		
		// Get the vertical and horizontal directions of the Entity
		glm::vec3 forward	= glm::vec3(0, 0,-1) * entity.mOrientation;
		glm::vec3 vAxis		= glm::vec3(0, 1, 0);
		glm::vec3 hAxis		= glm::cross(forward, vAxis);

		// Calculate the rotation around the Entity's x-axis
		float pitch			= -(mouseDelta.y / delta) * (1 / MOUSE_SPEED);
		if (pitch > glm::half_pi<float>()) { pitch = glm::half_pi<float>(); }
		if (pitch < -glm::half_pi<float>()) { pitch = -glm::half_pi<float>(); }
		glm::vec3 rotationX = pitch * hAxis;
		
		// Calculate the rotation around the Entity's y-axis
		float yaw			= -(mouseDelta.x / delta) * (1 / MOUSE_SPEED);
		glm::vec3 rotationY = yaw * vAxis;

		// Apply the change in orientation
		entity.mOrientation *= glm::normalize(glm::quat(rotationX + rotationY));

		// Reset the mouse position to the center of the screen
		mWindowSystem.setMousePosition(width / 2.0f, height / 2.0f);
	}


	void InputComponent::doKeyboardInput(
		Entity& entity,
		const window::InputData& inputData, float delta
	) const
	{
		glm::vec3 forward	= glm::vec3(0, 0,-1) * entity.mOrientation;
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
		glm::vec3 deltaP = RUN_SPEED * delta * direction;

		// Add the jump velocity
		if (inputData.mKeys[GLFW_KEY_SPACE]) { deltaP += JUMP_SPEED * delta * up; }
		if (inputData.mKeys[GLFW_KEY_LEFT_CONTROL]) { deltaP -= JUMP_SPEED * delta * up; }

		entity.mPosition += deltaP;
	}

}
