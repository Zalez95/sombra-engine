#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <se/window/KeyCodes.h>
#include <se/window/MouseButtonCodes.h>
#include <se/utils/Log.h>
#include <se/app/RawMesh.h>
#include <se/app/GraphicsManager.h>
#include <se/app/loaders/MeshLoader.h>
#include <se/graphics/3D/Mesh.h>
#include <se/graphics/3D/Material.h>
#include "PlayerController.h"
#include "Game.h"

extern bool PRINT;

namespace game {

	PlayerController::PlayerController(
		GameData& gameData, se::app::Entity& entity, se::graphics::RenderableText& pickText
	) : mGameData(gameData), mEntity(entity), mPickText(pickText),
		mYaw(0.0f), mPitch(0.0f), mMovement{}, mClicked(false)
	{
		mGameData.eventManager->subscribe(this, se::app::Topic::Key);
		mGameData.eventManager->subscribe(this, se::app::Topic::Mouse);

		se::app::RawMesh rawMesh2("tetrahedron");
		rawMesh2.positions = {
			{ 0.0f, 0.5f, 0.0f },
			{ 0.433012723f, -0.25f, 0.0f },
			{ -0.433012723f, -0.25f, 0.0f },
			{ 0.0f, 0.0f, 1.0f }
		};
		rawMesh2.texCoords = {
			{ 0.0f, 1.0f },
			{ 0.0f, 0.0f },
			{ 0.0f, 1.0f },
			{ 0.0f, 1.0f }
		};
		rawMesh2.faceIndices = {
			0, 1, 2,
			3, 1, 0,
			3, 2, 1,
			3, 0, 2
		};
		rawMesh2.normals = se::app::MeshLoader::calculateNormals(rawMesh2.positions, rawMesh2.faceIndices);
		rawMesh2.tangents = se::app::MeshLoader::calculateTangents(rawMesh2.positions, rawMesh2.texCoords, rawMesh2.faceIndices);
		mTetrahedronMesh = std::make_shared<se::graphics::Mesh>(se::app::MeshLoader::createGraphicsMesh(rawMesh2));

		mYellowMaterial = std::shared_ptr<se::graphics::Material>(new se::graphics::Material{
			"yellow_material",
			se::graphics::PBRMetallicRoughness{ { 1.0f, 1.0f, 0.0f, 1.0f }, nullptr, 0.2f, 0.5f, nullptr },
			nullptr, 1.0f, nullptr, 1.0f, nullptr, glm::vec3(0.0f), se::graphics::AlphaMode::Opaque, 0.5f, false
		});
	}


	PlayerController::~PlayerController()
	{
		mGameData.eventManager->unsubscribe(this, se::app::Topic::Mouse);
		mGameData.eventManager->unsubscribe(this, se::app::Topic::Key);
	}


	void PlayerController::update(float deltaTime)
	{
		mEntity.updated.reset( static_cast<int>(se::app::Entity::Update::Input) );
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

		if (mClicked) {
			std::string names;
			for (const auto& [entity, rayCast] : mGameData.collisionManager->getEntities(mEntity.position, forward)) {
				// Blue tetrahedron = separation direction from collider 1 to collider 0
				glm::vec3 new_z = rayCast.contactNormal;
				glm::vec3 new_x = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), new_z));
				glm::vec3 new_y = glm::normalize(glm::cross(new_z, new_x));

				auto pointEntity = new se::app::Entity(entity->name + "_rayCast");
				pointEntity->position = rayCast.contactPointWorld;
				pointEntity->orientation = glm::quat_cast(glm::mat4(glm::mat3(new_x, new_y, new_z)));

				auto r3d3 = std::make_unique<se::graphics::Renderable3D>(mTetrahedronMesh, mYellowMaterial);
				mGameData.graphicsManager->addRenderableEntity(pointEntity, std::move(r3d3));

				names += entity->name + "; ";
			}

			mPickText.setText(glm::to_string(mEntity.position) + " " + glm::to_string(forward) + " Selected entities: " + names);
			mClicked = false;
		}
	}


	void PlayerController::resetMousePosition()
	{
		SOMBRA_DEBUG_LOG << "Changing the mouse position to the center of the window";

		const se::window::WindowData& data = mGameData.windowSystem->getWindowData();
		mGameData.windowSystem->setMousePosition(data.width / 2.0, data.height / 2.0);
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
			const se::window::WindowData& data = mGameData.windowSystem->getWindowData();
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
				mClicked = true;
			}
		}
	}

}
