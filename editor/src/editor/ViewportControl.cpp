#include <se/app/EntityDatabase.h>
#include <se/app/TransformsComponent.h>
#include <se/window/WindowManager.h>
#include <se/window/KeyCodes.h>
#include <se/window/MouseButtonCodes.h>
#include "ViewportControl.h"

namespace editor {

	ViewportControl::ViewportControl(se::app::Application& application, se::app::Entity entity) :
		mApplication(application), mEntity(entity),
		mCtrlPressed(false), mShiftPressed(false), mAltPressed(false), mLastMousePositions{},
		mZoomActive(false), mMoveActive(false), mOrbitActive(false), mZoom(10.0f)
	{
		mApplication.getEventManager().subscribe(this, se::app::Topic::Key);
		mApplication.getEventManager().subscribe(this, se::app::Topic::MouseMove);
		mApplication.getEventManager().subscribe(this, se::app::Topic::MouseButton);
	}


	ViewportControl::~ViewportControl()
	{
		mApplication.getEventManager().unsubscribe(this, se::app::Topic::MouseButton);
		mApplication.getEventManager().unsubscribe(this, se::app::Topic::MouseMove);
		mApplication.getEventManager().unsubscribe(this, se::app::Topic::Key);
	}


	void ViewportControl::update()
	{
		auto& windowData = mApplication.getExternalTools().windowManager->getWindowData();
		auto [transforms] = mApplication.getEntityDatabase().getComponents<se::app::TransformsComponent>(mEntity);
		if (!transforms) { return; }

		transforms->updated.reset( static_cast<int>(se::app::TransformsComponent::Update::Input) );
		if (mZoomActive) {
			float zoomDelta = kMoveSpeed * (mLastMousePositions[0].y - mLastMousePositions[1].y) / windowData.height;
			float currentZoom = mZoom;
			float nextZoom = glm::max(currentZoom + zoomDelta, 0.0f);
			zoomDelta = nextZoom - currentZoom;

			mZoom = nextZoom;
			transforms->position -= zoomDelta * (transforms->orientation * glm::vec3(0.0f, 0.0f, 1.0f));
			transforms->updated.set(static_cast<int>(se::app::TransformsComponent::Update::Input));
		}
		else if (mMoveActive) {
			glm::vec2 windowSize = { windowData.width, windowData.height };
			glm::vec2 speed = {-kMoveSpeed, kMoveSpeed };
			glm::vec2 moveDelta = speed * (mLastMousePositions[0] - mLastMousePositions[1]) / windowSize;

			glm::vec3 front = glm::normalize(transforms->orientation * glm::vec3(0.0f, 0.0f, 1.0f));
			glm::vec3 up = { 0.0f, 1.0f, 0.0f };
			glm::vec3 right = glm::cross(front, up);
			up = glm::cross(right, front);

			transforms->position += moveDelta.x * right + moveDelta.y * up;
			transforms->updated.set(static_cast<int>(se::app::TransformsComponent::Update::Input));
		}
		else if (mOrbitActive) {
			glm::vec2 windowSize = { windowData.width, windowData.height };
			glm::vec2 mouseMove = (mLastMousePositions[0] - mLastMousePositions[1]) / windowSize;
			glm::vec3 front = glm::normalize(transforms->orientation * glm::vec3(0.0f, 0.0f, 1.0f));

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
			transforms->orientation = glm::normalize(transforms->orientation * qPitch * qYaw);

			// Apply the position
			transforms->position += mZoom * front;
			front = glm::normalize(transforms->orientation * glm::vec3(0.0f, 0.0f, 1.0f));
			transforms->position -= mZoom * front;

			transforms->updated.set( static_cast<int>(se::app::TransformsComponent::Update::Input) );
		}

		mLastMousePositions[1] = mLastMousePositions[0];
	}


	void ViewportControl::notify(const se::app::IEvent& event)
	{
		tryCall(&ViewportControl::onKeyEvent, event);
		tryCall(&ViewportControl::onMouseMoveEvent, event);
		tryCall(&ViewportControl::onMouseButtonEvent, event);
	}

// Private functions
	void ViewportControl::onKeyEvent(const se::app::KeyEvent& event)
	{
		switch (event.getKeyCode()) {
			case SE_KEY_LEFT_CONTROL:
				mCtrlPressed = (event.getState() != se::app::KeyEvent::State::Released);
				break;
			case SE_KEY_LEFT_SHIFT:
				mShiftPressed = (event.getState() != se::app::KeyEvent::State::Released);
				break;
			case SE_KEY_LEFT_ALT:
				mAltPressed = (event.getState() != se::app::KeyEvent::State::Released);
				break;
		}
	}


	void ViewportControl::onMouseMoveEvent(const se::app::MouseMoveEvent& event)
	{
		mLastMousePositions[0] = { static_cast<float>(event.getX()), static_cast<float>(event.getY()) };
	}


	void ViewportControl::onMouseButtonEvent(const se::app::MouseButtonEvent& event)
	{
		if (event.getButtonCode() == SE_MOUSE_BUTTON_LEFT) {
			switch (event.getState()) {
				case se::app::MouseButtonEvent::State::Pressed:
					if (mCtrlPressed && mAltPressed) {
						mZoomActive = true;
					}
					else if (mShiftPressed && mAltPressed) {
						mMoveActive = true;
					}
					else if (mAltPressed) {
						mOrbitActive = true;
					}
					break;
				case se::app::MouseButtonEvent::State::Released:
					mZoomActive = mMoveActive = mOrbitActive = false;
					break;
			}
		}
	}

}
