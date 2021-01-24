#include <se/app/EntityDatabase.h>
#include <se/app/TransformsComponent.h>
#include <se/window/WindowManager.h>
#include <se/window/KeyCodes.h>
#include <se/window/MouseButtonCodes.h>
#include "ViewportControl.h"

#define SPEED_X 25
#define SPEED_Y 25
#define SPEED_Z 50

namespace editor {

	ViewportControl::ViewportControl(se::app::Application& application, se::app::Entity entity) :
		mApplication(application), mEntity(entity),
		mCtrlPressed(false), mShiftPressed(false), mAltPressed(false),
		mZoom(false), mMove(false), mOrbit(false),
		mLastMousePositions{}
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

		if (mZoom) {
			float newZoom = SPEED_Z * (mLastMousePositions[0].y - mLastMousePositions[1].y) / windowData.height;
			transforms->position += newZoom * (transforms->orientation * glm::vec3(0.0f, 0.0f, 1.0f));
			transforms->updated.set(static_cast<int>(se::app::TransformsComponent::Update::Input));
		}
		else if (mMove) {
			glm::vec2 windowSize = { windowData.width, windowData.height };
			glm::vec2 speed = {-SPEED_X, SPEED_Y };
			glm::vec2 newMove = speed * (mLastMousePositions[0] - mLastMousePositions[1]) / windowSize;

			glm::vec3 front = glm::normalize(transforms->orientation * glm::vec3(0.0f, 0.0f, 1.0f));
			glm::vec3 up = { 0.0f, 1.0f, 0.0f };
			glm::vec3 right = glm::cross(front, up);
			up = glm::cross(right, front);

			transforms->position += newMove.x * right + newMove.y * up;
			transforms->updated.set(static_cast<int>(se::app::TransformsComponent::Update::Input));
		}
		else if (mOrbit) {
			auto calcArcball = [&](const glm::vec2& p) {
				glm::vec2 windowSize = { windowData.width, windowData.height };
				glm::vec3 v = glm::vec3(2.0f * p / windowSize - glm::vec2(1.0f), 0.0f);
				float l2 = glm::dot(v, v);
				if (l2 <= 1.0f) {
					v.z = glm::sqrt(1.0f - l2);
				}
				else {
					v *= glm::inversesqrt(l2);
				}
				return v;
			};

			glm::vec3 v0 = calcArcball(mLastMousePositions[0]);
			glm::vec3 v1 = calcArcball(mLastMousePositions[1]);
			float angle = std::acos( std::min(1.0f, glm::dot(v1, v0)) );
			glm::vec3 axis = glm::cross(v1, v0);
			float lAxis = glm::dot(axis, axis);
			if (lAxis > 0) {
				axis *= glm::inversesqrt(lAxis);
			}

			glm::quat rotation = glm::angleAxis(angle, axis);
			glm::vec3 frontV = glm::normalize(transforms->orientation * glm::vec3(0.0f, 0.0f, 1.0f));
			glm::vec3 frontP = transforms->position + 10.0f * frontV;
			transforms->position = frontP + rotation * (-10.0f * frontV);
			transforms->orientation = glm::normalize(transforms->orientation * rotation);

			transforms->updated.set(static_cast<int>(se::app::TransformsComponent::Update::Input));
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
						mZoom = true;
					}
					else if (mShiftPressed && mAltPressed) {
						mMove = true;
					}
					else if (mAltPressed) {
						mOrbit = true;
					}
					break;
				case se::app::MouseButtonEvent::State::Released:
					mZoom = mMove = mOrbit = false;
					break;
			}
		}
	}

}
