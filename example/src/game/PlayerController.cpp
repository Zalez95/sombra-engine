#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <se/window/KeyCodes.h>
#include <se/window/MouseButtonCodes.h>
#include <se/utils/Log.h>
#include <se/utils/Repository.h>
#include <se/window/WindowManager.h>
#include <se/graphics/Renderer.h>
#include <se/graphics/GraphicsEngine.h>
#include <se/app/EntityDatabase.h>
#include <se/app/TagComponent.h>
#include <se/app/MeshComponent.h>
#include <se/app/TransformsComponent.h>
#include <se/app/CameraSystem.h>
#include <se/app/CollisionSystem.h>
#include <se/app/Scene.h>
#include <se/app/io/MeshLoader.h>
#include <se/app/io/ShaderLoader.h>
#include "PlayerController.h"
#include "Game.h"
#include "Level.h"

extern bool PRINT;

namespace game {

	PlayerController::PlayerController(Level& level, se::graphics::RenderableText& pickText) :
		mLevel(level), mPickText(pickText),
		mYaw(0.0f), mPitch(0.0f), mMovement{}, mClicked(false)
	{
		mLevel.getGame().getEventManager().subscribe(this, se::app::Topic::Key);
		mLevel.getGame().getEventManager().subscribe(this, se::app::Topic::MouseMove);
		mLevel.getGame().getEventManager().subscribe(this, se::app::Topic::MouseButton);

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
		rawMesh2.indices = {
			0, 1, 2,
			3, 1, 0,
			3, 2, 1,
			3, 0, 2
		};
		rawMesh2.normals = se::app::MeshLoader::calculateNormals(rawMesh2.positions, rawMesh2.indices);
		rawMesh2.tangents = se::app::MeshLoader::calculateTangents(rawMesh2.positions, rawMesh2.texCoords, rawMesh2.indices);
		mTetrahedronMesh = std::make_shared<se::graphics::Mesh>(se::app::MeshLoader::createGraphicsMesh(rawMesh2));

		auto& scene = mLevel.getScene();
		auto gBufferRenderer = static_cast<se::graphics::Renderer*>(mLevel.getGame().getExternalTools().graphicsEngine->getRenderGraph().getNode("gBufferRenderer"));

		auto programGBufMaterial = scene.repository.find<std::string, se::graphics::Program>("programGBufMaterial");
		auto passYellow = std::make_shared<se::graphics::Pass>(*gBufferRenderer);
		scene.repository.add(std::string("passYellow"), passYellow);

		se::app::ShaderLoader::addMaterialBindables(
			passYellow,
			se::app::Material{
				se::app::PBRMetallicRoughness{ { 1.0f, 1.0f, 0.0f, 1.0f }, {}, 0.2f, 0.5f, {} },
				{}, 1.0f, {}, 1.0f, {}, glm::vec3(0.0f), se::graphics::AlphaMode::Opaque, 0.5f, false
			},
			programGBufMaterial
		);

		mYellowTechnique = std::make_shared<se::graphics::Technique>();
		mYellowTechnique->addPass(passYellow);
	}


	PlayerController::~PlayerController()
	{
		mLevel.getGame().getEventManager().unsubscribe(this, se::app::Topic::MouseButton);
		mLevel.getGame().getEventManager().unsubscribe(this, se::app::Topic::MouseMove);
		mLevel.getGame().getEventManager().unsubscribe(this, se::app::Topic::Key);
	}


	void PlayerController::update(float deltaTime)
	{
		auto [transforms] = mLevel.getGame().getEntityDatabase().getComponents<se::app::TransformsComponent>(mLevel.getPlayer());
		transforms->updated.reset( static_cast<int>(se::app::TransformsComponent::Update::Input) );
		glm::vec3 forward	= glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation;
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
			SOMBRA_DEBUG_LOG << "Updating the entity " << mLevel.getPlayer() << " orientation (" << pitch << ", " << yaw << ")";

			// Apply the rotation
			glm::quat qYaw = glm::angleAxis(yaw, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::quat qPitch = glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));
			transforms->orientation = glm::normalize(qPitch * transforms->orientation * qYaw);
			transforms->updated.set( static_cast<int>(se::app::TransformsComponent::Update::Input) );
		}

		// Add WASD movement
		glm::vec3 direction(0.0f);
		if (mMovement[static_cast<int>(Direction::Front)]) { direction += forward; }
		if (mMovement[static_cast<int>(Direction::Back)]) { direction -= forward; }
		if (mMovement[static_cast<int>(Direction::Right)]) { direction += right; }
		if (mMovement[static_cast<int>(Direction::Left)]) { direction -= right; }
		float length = glm::length(direction);
		if (length > 0.0f) {
			transforms->velocity += kRunSpeed * direction / length;
			SOMBRA_DEBUG_LOG << "Updating the entity " << mLevel.getPlayer() << " run velocity (" << glm::to_string(transforms->velocity) << ")";
			transforms->updated.set( static_cast<int>(se::app::TransformsComponent::Update::Input) );
		}

		// Add the world Y velocity
		direction = glm::vec3(0.0f);
		if (mMovement[static_cast<int>(Direction::Up)]) { direction += up; }
		if (mMovement[static_cast<int>(Direction::Down)]) { direction -= up; }
		length = glm::length(direction);
		if (length > 0.0f) {
			transforms->velocity += kJumpSpeed * direction;
			SOMBRA_DEBUG_LOG << "Updating the entity " << mLevel.getPlayer() << " jump velocity (" << glm::to_string(transforms->velocity) << ")";
			transforms->updated.set( static_cast<int>(se::app::TransformsComponent::Update::Input) );
		}

		/*if (mClicked) {
			std::string names;
			for (const auto& [entity, rayCast] : mGame.getExternalTools(). collisionSystem->getEntities(transforms->position, forward)) {
				// Blue tetrahedron = separation direction from collider 1 to collider 0
				glm::vec3 new_z = rayCast.contactNormal;
				glm::vec3 new_x = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), new_z));
				glm::vec3 new_y = glm::normalize(glm::cross(new_z, new_x));

				auto pointEntity = mGame.getEntityDatabase().addEntity();

				se::app::TransformsComponent transforms;
				transforms.position = rayCast.contactPointWorld;
				transforms.orientation = glm::quat_cast(glm::mat4(glm::mat3(new_x, new_y, new_z)));
				mGame.getEntityDatabase().addComponent(pointEntity, std::move(transforms));

				se::app::MeshComponent mesh;
				mesh.rMeshes.emplace_back(mTetrahedronMesh).addTechnique(mYellowTechnique);
				mGame.getEntityDatabase().addComponent(pointEntity, std::move(mesh));

				auto [tag] = mGame.getEntityDatabase().getComponents<se::app::TagComponent>(entity);
				names += std::string(tag->getName()) + "; ";
			}

			mPickText.setText(glm::to_string(transforms->position) + " " + glm::to_string(forward) + " Selected entities: " + names);
			mClicked = false;
		}*/
	}


	void PlayerController::resetMousePosition()
	{
		SOMBRA_DEBUG_LOG << "Changing the mouse position to the center of the window";

		auto windowManager = mLevel.getGame().getExternalTools().windowManager;
		const se::window::WindowData& data = windowManager->getWindowData();
		windowManager->setMousePosition(data.width / 2.0, data.height / 2.0);
	}


	void PlayerController::notify(const se::app::IEvent& event)
	{
		tryCall(&PlayerController::onKeyEvent, event);
		tryCall(&PlayerController::onMouseMoveEvent, event);
		tryCall(&PlayerController::onMouseButtonEvent, event);
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


	void PlayerController::onMouseMoveEvent(const se::app::MouseMoveEvent& event)
	{
		// Get the mouse movement from the center of the screen in the range [-1, 1]
		const se::window::WindowData& data = mLevel.getGame().getExternalTools().windowManager->getWindowData();
		double mouseDeltaX = 2.0 * event.getX() / data.width - 1.0;
		double mouseDeltaY = 1.0 - 2.0 * event.getY() / data.height;	// note that the Y position is upsidedown

		// Multiply the values by the mouse speed
		mYaw = kMouseSpeed * static_cast<float>(mouseDeltaX);
		mPitch = kMouseSpeed * static_cast<float>(mouseDeltaY);

		resetMousePosition();
	}


	void PlayerController::onMouseButtonEvent(const se::app::MouseButtonEvent& event)
	{
		if ((event.getState() == se::app::MouseButtonEvent::State::Pressed)
			&& (event.getButtonCode() == SE_MOUSE_BUTTON_LEFT)
		) {
			mClicked = true;
		}
	}

}
