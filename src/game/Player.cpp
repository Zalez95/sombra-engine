#include "Player.h"
#include <GLFW/glfw3.h>
#include "../window/InputData.h"

namespace game {

// Static variables intialization
	const float Player::STEP_LENGTH			= 0.15f;
	const float Player::JUMP_HEIGHT			= 0.2f;
	const float Player::MOUSE_SENSIBILITY	= 10.0f;

// Public functions
	void Player::doInput(const window::InputData* inputData, float delta)
	{
		doMouseInput(inputData, delta);
		doKeyboardInput(inputData, delta);
	}

// Private functions
	void Player::doMouseInput(const window::InputData* inputData, float delta)
	{
		glm::vec2 mousePosition(inputData->mMouseX, inputData->mMouseY);
		glm::vec2 mouseDelta = mousePosition / mWindowDimensions - glm::vec2(0.5f, 0.5f);

		float yaw	= glm::radians(MOUSE_SENSIBILITY * mouseDelta.x) / delta;
		float pitch = glm::radians(MOUSE_SENSIBILITY * mouseDelta.y) / delta;

		physics::RigidBody* rigidBody = mPhysicsEntity->getRigidBody();
		glm::vec3 angularVelocity;// = rigidBody->getAngularVelocity();

		// Rotate around the y-axis
		glm::vec3 vAxis = mDefaultUpVector;
		angularVelocity += yaw * vAxis;

		// Rotate around the x-axis
		glm::vec3 direction = mDefaultForwardVector * rigidBody->getOrientation();
		glm::vec3 hVector = glm::cross(direction, vAxis);
		angularVelocity += pitch * hVector;

		rigidBody->setAngularVelocity(angularVelocity);
	}


	void Player::doKeyboardInput(const window::InputData* inputData, float delta)
	{
		glm::vec3 forward	= mDefaultForwardVector;
		glm::vec3 up		= mDefaultUpVector;
		glm::vec3 right		= glm::cross(forward, up);
		
		// Get the displacement of the input in the XZ plane
		glm::vec3 displacement;
		if (inputData->mKeys[GLFW_KEY_W]) { displacement += forward; }
		if (inputData->mKeys[GLFW_KEY_S]) { displacement -= forward; }
		if (inputData->mKeys[GLFW_KEY_D]) { displacement += right; }
		if (inputData->mKeys[GLFW_KEY_A]) { displacement -= right; }
		
		// Rotate the displacement
		physics::RigidBody* rigidBody = mPhysicsEntity->getRigidBody();
		displacement = displacement * rigidBody->getOrientation();

		// Normalize the displacement
		float len = glm::length(displacement);
		if (len > 0) { displacement /= len; }

		// Transform the displacement to velocity
		glm::vec3 velocity = rigidBody->getVelocity() + STEP_LENGTH * displacement / delta;

		// Add the jump velocity
		if (inputData->mKeys[GLFW_KEY_SPACE]) { velocity += up * JUMP_HEIGHT / delta; }
		if (inputData->mKeys[GLFW_KEY_LEFT_CONTROL]) { velocity -= up * JUMP_HEIGHT / delta; }

		mPhysicsEntity->getRigidBody()->setVelocity(velocity);
	}

}
