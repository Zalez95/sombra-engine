#include "Player.h"
#include <GLFW/glfw3.h>
#include "../window/InputData.h"

namespace game {

// Static variables intialization
	const float Player::RUN_SPEED	= 100.0f;
	const float Player::JUMP_SPEED	= 50.0f;
	const float Player::MOUSE_SPEED	= 5.0f;

// Public functions
	void Player::doInput(const window::InputData* inputData, float delta)
	{
		doMouseInput(inputData, delta);
		doKeyboardInput(inputData, delta);
	}

// Private functions
	void Player::doMouseInput(const window::InputData* inputData, float delta)
	{
		physics::RigidBody* rigidBody = mPhysicsEntity->getRigidBody();
		
		glm::vec2 mouseDelta;
		mouseDelta.x		= inputData->mMouseX / mWindowDimensions.x - 0.5f;
		mouseDelta.y		= inputData->mMouseY / mWindowDimensions.y - 0.5f;

		// Calculate the change in the orientation
		float yaw			= MOUSE_SPEED * delta * mouseDelta.x;
		float pitch			= MOUSE_SPEED * delta * mouseDelta.y;

		glm::vec3 direction	= mDefaultForwardVector * rigidBody->getOrientation();
		glm::vec3 vAxis		= mDefaultUpVector;
		glm::vec3 hAxis		= glm::cross(direction, vAxis);
		
		// Rotate around the x-axis
		glm::vec3 rotation	= pitch * hAxis;
		
		// Rotate around the y-axis
		rotation			+= yaw * vAxis;

		// Apply the change in orientation
		rigidBody->addAngularVelocity(rotation);
	}


	void Player::doKeyboardInput(const window::InputData* inputData, float delta)
	{
		physics::RigidBody* rigidBody = mPhysicsEntity->getRigidBody();
		glm::vec3 forward	= mDefaultForwardVector * rigidBody->getOrientation();
		glm::vec3 up		= mDefaultUpVector;
		glm::vec3 right		= glm::cross(forward, up);
		
		// Get the direction from the input in the XZ plane
		glm::vec3 direction;
		if (inputData->mKeys[GLFW_KEY_W]) { direction += forward; }
		if (inputData->mKeys[GLFW_KEY_S]) { direction -= forward; }
		if (inputData->mKeys[GLFW_KEY_D]) { direction += right; }
		if (inputData->mKeys[GLFW_KEY_A]) { direction -= right; }

		// Normalize the direction
		float len = glm::length(direction);
		if (len > 0) { direction /= len; }

		// Transform the direction to velocity
		glm::vec3 velocity = RUN_SPEED * delta * direction;

		// Add the jump velocity
		if (inputData->mKeys[GLFW_KEY_SPACE]) { velocity += JUMP_SPEED * delta * up; }
		if (inputData->mKeys[GLFW_KEY_LEFT_CONTROL]) { velocity -= JUMP_SPEED * delta * up; }

		rigidBody->addLinearVelocity(velocity);
	}

}
