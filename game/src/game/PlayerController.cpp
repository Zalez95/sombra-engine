#include <se/window/KeyCodes.h>
#include <se/window/MouseButtonCodes.h>
#include <se/utils/Log.h>
#include <glm/gtx/string_cast.hpp>
#include "PlayerController.h"

extern bool PRINT;

namespace game {

	PlayerController::PlayerController(
		se::app::Entity& entity, se::app::EventManager& eventManager, se::window::WindowSystem& windowSystem,
		se::app::CollisionManager& collisionManager
	) : mEntity(entity), mEventManager(eventManager), mWindowSystem(windowSystem), mCollisionManager(collisionManager), mYaw(0.0f), mPitch(0.0f), mMovement{}
	{
		mEventManager.subscribe(this, se::app::Topic::Key);
		mEventManager.subscribe(this, se::app::Topic::Mouse);
	}


	PlayerController::~PlayerController()
	{
		mEventManager.unsubscribe(this, se::app::Topic::Mouse);
		mEventManager.unsubscribe(this, se::app::Topic::Key);
	}


	void PlayerController::update(float deltaTime)
	{
		glm::vec3 forward	= glm::vec3(0.0f, 0.0f, 1.0f) * mEntity.orientation;
		glm::vec3 up		= glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 right		= glm::cross(forward, up);

		// Set the pitch and yaw
		if ((mYaw != 0.0f) || (mPitch != 0.0f)) {
			float yaw = mYaw * deltaTime;
			float pitch = mPitch * deltaTime;
			mYaw = mPitch = 0.0f;

			// Clamp the pitch
			float currentPitch = std::asin(forward.y);
			float nextPitch = currentPitch + pitch;
			nextPitch = std::clamp(nextPitch, -glm::half_pi<float>() + kPitchLimit, glm::half_pi<float>() - kPitchLimit);
			pitch = nextPitch - currentPitch;
			SOMBRA_DEBUG_LOG << "Updating the entity " << &mEntity << " orientation (" << pitch << ", " << yaw << ")";

			// Apply the rotation
			glm::quat qYaw = glm::angleAxis(yaw, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::quat qPitch = glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));
			mEntity.orientation = glm::normalize(qPitch * mEntity.orientation * qYaw);
			mEntity.updated.set( static_cast<int>(se::app::Entity::Update::Input) );
		}

		// Add WASD movement
		glm::vec3 direction(0.0f);
		if (mMovement[static_cast<int>(Direction::Front)]) { direction += forward; }
		if (mMovement[static_cast<int>(Direction::Back)]) { direction -= forward; }
		if (mMovement[static_cast<int>(Direction::Right)]) { direction += right; }
		if (mMovement[static_cast<int>(Direction::Left)]) { direction -= right; }
		float length = glm::length(direction);
		if (length > 0.0f) {
			mEntity.velocity += kRunSpeed * direction / length;
			SOMBRA_DEBUG_LOG << "Updating the entity " << &mEntity << " run velocity (" << glm::to_string(mEntity.velocity) << ")";
			mEntity.updated.set( static_cast<int>(se::app::Entity::Update::Input) );
		}

		// Add the world Y velocity
		direction = glm::vec3(0.0f);
		if (mMovement[static_cast<int>(Direction::Up)]) { direction += up; }
		if (mMovement[static_cast<int>(Direction::Down)]) { direction -= up; }
		length = glm::length(direction);
		if (length > 0.0f) {
			mEntity.velocity += kJumpSpeed * direction;
			SOMBRA_DEBUG_LOG << "Updating the entity " << &mEntity << " jump velocity (" << glm::to_string(mEntity.velocity) << ")";
			mEntity.updated.set( static_cast<int>(se::app::Entity::Update::Input) );
		}
	}


	void PlayerController::resetMousePosition()
	{
		SOMBRA_DEBUG_LOG << "Changing the mouse position to the center of the window";

		const se::window::WindowData& data = mWindowSystem.getWindowData();
		mWindowSystem.setMousePosition(data.width / 2.0, data.height / 2.0);
	}


	void PlayerController::notify(const se::app::IEvent& event)
	{
		tryCall(&PlayerController::onKeyEvent, event);
		tryCall(&PlayerController::onMouseEvent, event);
	}

// Private functions
	void PlayerController::onKeyEvent(const se::app::KeyEvent& event)
	{
		switch (event.getKeyCode()) {
			case SE_KEY_W:
				mMovement[static_cast<int>(Direction::Front)] = (event.getState() != se::app::KeyEvent::State::Released);
				break;
			case SE_KEY_A:
				mMovement[static_cast<int>(Direction::Left)] = (event.getState() != se::app::KeyEvent::State::Released);
				break;
			case SE_KEY_S:
				mMovement[static_cast<int>(Direction::Back)] = (event.getState() != se::app::KeyEvent::State::Released);
				break;
			case SE_KEY_D:
				mMovement[static_cast<int>(Direction::Right)] = (event.getState() != se::app::KeyEvent::State::Released);
				break;
			case SE_KEY_SPACE:
				mMovement[static_cast<int>(Direction::Up)] = (event.getState() != se::app::KeyEvent::State::Released);
				break;
			case SE_KEY_P:
				if (event.getState() != se::app::KeyEvent::State::Released) {
					PRINT = !PRINT;
				}
				break;
			case SE_KEY_LEFT_CONTROL:
				mMovement[static_cast<int>(Direction::Down)] = (event.getState() != se::app::KeyEvent::State::Released);
				break;
			default:
				break;
		}
	}


	void PlayerController::onMouseEvent(const se::app::MouseEvent& event)
	{
		if (event.getType() == se::app::MouseEvent::Type::Move) {
			auto moveEvent = static_cast<const se::app::MouseMoveEvent&>(event);

			// Get the mouse movement from the center of the screen in the range [-1, 1]
			const se::window::WindowData& data = mWindowSystem.getWindowData();
			double mouseDeltaX = 2.0 * moveEvent.getX() / data.width - 1.0;
			double mouseDeltaY = 1.0 - 2.0 * moveEvent.getY() / data.height;	// note that the Y position is upsidedown

			// Multiply the values by the mouse speed
			mYaw = kMouseSpeed * static_cast<float>(mouseDeltaX);
			mPitch = kMouseSpeed * static_cast<float>(mouseDeltaY);

			resetMousePosition();
		}
		else if (event.getType() == se::app::MouseEvent::Type::ButtonPressed) {
			auto buttonEvent = static_cast<const se::app::MouseButtonEvent&>(event);

			if (buttonEvent.getButtonCode() == SE_MOUSE_BUTTON_LEFT) {
				glm::vec3 forward = mEntity.orientation * glm::vec3(0.0f, 0.0f, 1.0f);
				std::string entityName = mCollisionManager.getName(mEntity.position, forward);
				SOMBRA_INFO_LOG << "Selected entity: " << entityName;
			}
		}
	}

}
