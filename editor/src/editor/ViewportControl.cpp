#include <algorithm>
#include <se/window/WindowManager.h>
#include <se/window/KeyCodes.h>
#include <se/window/MouseButtonCodes.h>
#include "ViewportControl.h"

namespace editor {

	void ViewportControl::onUpdate(se::app::Entity entity, float /*elapsedTime*/, const se::app::ScriptSharedState& sharedState)
	{
		sharedState.entityDatabase->executeQuery([&](se::app::EntityDatabase::Query& query) {
			auto [transforms] = query.getComponents<se::app::TransformsComponent>(entity, true);
			if (transforms && sharedState.mouseButtons[SE_MOUSE_BUTTON_LEFT]) {
				if (sharedState.keys[SE_KEY_LEFT_CONTROL] && sharedState.keys[SE_KEY_LEFT_ALT]) {
					zoom(sharedState, *transforms);
				}
				else if (sharedState.keys[SE_KEY_LEFT_SHIFT] && sharedState.keys[SE_KEY_LEFT_ALT]) {
					move(sharedState, *transforms);
				}
				else if (sharedState.keys[SE_KEY_LEFT_ALT]) {
					orbit(sharedState, *transforms);
				}
			}
		});

		mLastMouseX = sharedState.mouseX;
		mLastMouseY = sharedState.mouseY;
	}


	std::unique_ptr<se::app::Script> ViewportControl::clone() const
	{
		return std::make_unique<ViewportControl>(*this);
	}

// Private functions
	void ViewportControl::zoom(const se::app::ScriptSharedState& sharedState, se::app::TransformsComponent& transforms)
	{
		float zoomDelta = kMoveSpeed * (sharedState.mouseY - mLastMouseY) / sharedState.windowHeight;
		float currentZoom = mZoom;
		float nextZoom = glm::max(currentZoom + zoomDelta, 0.0f);
		zoomDelta = nextZoom - currentZoom;

		mZoom = nextZoom;
		transforms.position += zoomDelta * (transforms.orientation * glm::vec3(0.0f, 0.0f, 1.0f));
		transforms.updated.reset();
	}


	void ViewportControl::move(const se::app::ScriptSharedState& sharedState, se::app::TransformsComponent& transforms)
	{
		glm::vec2 windowSize = { sharedState.windowWidth, sharedState.windowHeight };
		glm::vec2 speed = { kMoveSpeed, kMoveSpeed };
		glm::vec2 moveDelta = glm::vec2(sharedState.mouseX, sharedState.mouseY) - glm::vec2(mLastMouseX, mLastMouseY);
		moveDelta = speed * moveDelta / windowSize;

		glm::vec3 front = glm::normalize(transforms.orientation * glm::vec3(0.0f, 0.0f, 1.0f));
		glm::vec3 up = glm::normalize(transforms.orientation * glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 right = glm::cross(front, up);

		transforms.position += moveDelta.x * right + moveDelta.y * up;
		transforms.updated.reset();
	}


	void ViewportControl::orbit(const se::app::ScriptSharedState& sharedState, se::app::TransformsComponent& transforms)
	{
		glm::vec2 windowSize = { sharedState.windowWidth, sharedState.windowHeight };
		glm::vec2 mouseMove = glm::vec2(sharedState.mouseX, sharedState.mouseY) - glm::vec2(mLastMouseX, mLastMouseY);
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
