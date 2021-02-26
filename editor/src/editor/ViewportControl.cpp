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

		transforms->updated.reset( static_cast<int>(se::app::TransformsComponent::Update::Input) );

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
		transforms.position -= zoomDelta * (transforms.orientation * glm::vec3(0.0f, 0.0f, 1.0f));
		transforms.updated.set(static_cast<int>(se::app::TransformsComponent::Update::Input));
	}


	void ViewportControl::move(const se::app::UserInput& userInput, se::app::TransformsComponent& transforms)
	{
			glm::vec2 windowSize = {userInput.windowWidth, userInput.windowHeight };
			glm::vec2 speed = {-kMoveSpeed, kMoveSpeed };
			glm::vec2 moveDelta = glm::vec2(userInput.mouseX, userInput.mouseY) - glm::vec2(mLastMouseX, mLastMouseY);
			moveDelta = speed * moveDelta / windowSize;

			glm::vec3 front = glm::normalize(transforms.orientation * glm::vec3(0.0f, 0.0f, 1.0f));
			glm::vec3 up = { 0.0f, 1.0f, 0.0f };
			glm::vec3 right = glm::cross(front, up);
			up = glm::cross(right, front);

			transforms.position += moveDelta.x * right + moveDelta.y * up;
			transforms.updated.set(static_cast<int>(se::app::TransformsComponent::Update::Input));
	}


	void ViewportControl::orbit(const se::app::UserInput& userInput, se::app::TransformsComponent& transforms)
	{
		glm::vec2 windowSize = {userInput.windowWidth, userInput.windowHeight };
		glm::vec2 mouseMove = glm::vec2(userInput.mouseX, userInput.mouseY) - glm::vec2(mLastMouseX, mLastMouseY);
		mouseMove /= windowSize;
		glm::vec3 front = glm::normalize(transforms.orientation * glm::vec3(0.0f, 0.0f, 1.0f));

		// Yaw
		float yaw = -kRotationSpeed * mouseMove.x;

		// Pitch
		float currentPitch = std::asin(front.y);
		float nextPitch = currentPitch + kRotationSpeed * mouseMove.y;
		nextPitch = std::clamp(nextPitch, -glm::half_pi<float>() + kPitchLimit, glm::half_pi<float>() - kPitchLimit);
		float pitch = nextPitch - currentPitch;

		// Apply the rotation
		glm::quat qYaw = glm::angleAxis(yaw, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::quat qPitch = glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));
		transforms.orientation = glm::normalize(transforms.orientation * qPitch * qYaw);

		// Apply the position
		transforms.position += mZoom * front;
		front = glm::normalize(transforms.orientation * glm::vec3(0.0f, 0.0f, 1.0f));
		transforms.position -= mZoom * front;

		transforms.updated.set( static_cast<int>(se::app::TransformsComponent::Update::Input) );
	}

}
