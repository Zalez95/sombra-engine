#include <algorithm>
#include <se/app/EntityDatabase.h>
#include <se/window/WindowManager.h>
#include <se/window/KeyCodes.h>
#include <se/window/MouseButtonCodes.h>
#include "ViewportControl.h"

namespace editor {

	void ViewportControl::onUpdate(float /*elapsedTime*/, const se::app::UserInput& userInput)
	{
		auto [transforms] = mEntityDatabase->getComponents<se::app::TransformsComponent>(mEntity);
		if (!transforms) { return; }

		if (userInput.mouseButtons[SE_MOUSE_BUTTON_LEFT]) {
			if (userInput.keys[SE_KEY_LEFT_CONTROL] && userInput.keys[SE_KEY_LEFT_ALT]) {
				zoom(userInput, *transforms);
			}
			else if (userInput.keys[SE_KEY_LEFT_SHIFT] && userInput.keys[SE_KEY_LEFT_ALT]) {
				move(userInput, *transforms);
			}
			else if (userInput.keys[SE_KEY_LEFT_ALT]) {
				orbit(userInput, *transforms);
			}
		}

		mLastMouseX = userInput.mouseX;
		mLastMouseY = userInput.mouseY;
	}

// Private functions
	void ViewportControl::zoom(const se::app::UserInput& userInput, se::app::TransformsComponent& transforms)
	{
		float zoomDelta = kMoveSpeed * (userInput.mouseY - mLastMouseY) / userInput.windowHeight;
		float currentZoom = mZoom;
		float nextZoom = glm::max(currentZoom + zoomDelta, 0.0f);
		zoomDelta = nextZoom - currentZoom;

		mZoom = nextZoom;
		transforms.position += zoomDelta * (transforms.orientation * glm::vec3(0.0f, 0.0f, 1.0f));
		transforms.updated.reset();
	}


	void ViewportControl::move(const se::app::UserInput& userInput, se::app::TransformsComponent& transforms)
	{
		glm::vec2 windowSize = {userInput.windowWidth, userInput.windowHeight };
		glm::vec2 speed = { kMoveSpeed, kMoveSpeed };
		glm::vec2 moveDelta = glm::vec2(userInput.mouseX, userInput.mouseY) - glm::vec2(mLastMouseX, mLastMouseY);
		moveDelta = speed * moveDelta / windowSize;

		glm::vec3 front = glm::normalize(transforms.orientation * glm::vec3(0.0f, 0.0f, 1.0f));
		glm::vec3 up = glm::normalize(transforms.orientation * glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 right = glm::cross(front, up);

		transforms.position += moveDelta.x * right + moveDelta.y * up;
		transforms.updated.reset();
	}


	void ViewportControl::orbit(const se::app::UserInput& userInput, se::app::TransformsComponent& transforms)
	{
		glm::vec2 windowSize = {userInput.windowWidth, userInput.windowHeight };
		glm::vec2 mouseMove = glm::vec2(userInput.mouseX, userInput.mouseY) - glm::vec2(mLastMouseX, mLastMouseY);
		mouseMove /= windowSize;
		glm::vec3 front = glm::normalize(transforms.orientation * glm::vec3(0.0f, 0.0f, 1.0f));

		// Apply the rotation
		float yaw = -kRotationSpeed * mouseMove.x;
		float pitch = -kRotationSpeed * mouseMove.y;
		glm::quat qYaw = glm::angleAxis(yaw, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::quat qPitch = glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));
		transforms.orientation = glm::normalize(qYaw * transforms.orientation * qPitch);

		// Apply the position
		transforms.position -= mZoom * front;
		front = glm::normalize(transforms.orientation * glm::vec3(0.0f, 0.0f, 1.0f));
		transforms.position += mZoom * front;

		transforms.updated.reset();
	}

}
