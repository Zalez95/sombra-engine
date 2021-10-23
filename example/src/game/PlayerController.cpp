#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <se/window/KeyCodes.h>
#include <se/window/MouseButtonCodes.h>
#include <se/utils/Log.h>
#include <se/app/Repository.h>
#include <se/graphics/Renderer.h>
#include <se/graphics/GraphicsEngine.h>
#include <se/app/TagComponent.h>
#include <se/app/MeshComponent.h>
#include <se/app/TransformsComponent.h>
#include <se/app/Scene.h>
#include <se/app/io/MeshLoader.h>
#include <se/app/io/ShaderLoader.h>
#include <se/app/events/MouseEvents.h>
#include "PlayerController.h"
#include "Game.h"
#include "Level.h"

extern bool PRINT;

namespace game {

	PlayerController::PlayerController(Level& level, se::graphics::RenderableText& pickText) :
		mLevel(level), mPickText(pickText)
	{
		auto& scene = mLevel.getScene();
		auto gBufferRenderer = static_cast<se::graphics::Renderer*>(mLevel.getGame().getExternalTools().graphicsEngine->getRenderGraph().getNode("gBufferRendererMesh"));

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
		mTetrahedronMesh = scene.repository.insert(std::make_shared<se::graphics::Mesh>(se::app::MeshLoader::createGraphicsMesh(rawMesh2)), "tetrahedronMesh");

		auto programGBufMaterial = scene.repository.findByName<se::graphics::Program>("programGBufMaterial");
		auto stepYellow = scene.repository.insert(std::make_shared<se::app::RenderableShaderStep>(*gBufferRenderer), "stepYellow");

		se::app::ShaderLoader::addMaterialBindables(
			stepYellow,
			se::app::Material{
				se::app::PBRMetallicRoughness{ { 1.0f, 1.0f, 0.0f, 1.0f }, {}, 0.2f, 0.5f, {} },
				{}, 1.0f, {}, 1.0f, {}, glm::vec3(0.0f), se::graphics::AlphaMode::Opaque, 0.5f, false
			},
			programGBufMaterial
		);

		mShaderYellow = scene.repository.insert(std::make_shared<se::app::RenderableShader>(mLevel.getGame().getEventManager()), "shaderYellow");
		mShaderYellow->addStep(stepYellow);

		mLightYellow = scene.repository.emplace<se::app::LightSource>(mLevel.getGame().getEventManager(), se::app::LightSource::Type::Point);
		mLightYellow.getResource().setName("yellow");
		mLightYellow->setColor({ 1.0f, 1.0f, 0.0f });
	}


	void PlayerController::onCreate(const se::app::UserInput& userInput)
	{
		mLastMouseX = 0.5f * userInput.windowWidth;
		mLastMouseY = 0.5f * userInput.windowHeight;
	}


	void PlayerController::onUpdate(float elapsedTime, const se::app::UserInput& userInput)
	{
		auto [transforms] = mEntityDatabase->getComponents<se::app::TransformsComponent>(mEntity, true);
		if (!transforms) { return; }

		// Get the mouse movement
		glm::vec2 windowSize = {userInput.windowWidth, userInput.windowHeight };
		glm::vec2 mouseMove = glm::vec2(userInput.mouseX, userInput.mouseY) - glm::vec2(mLastMouseX, mLastMouseY);
		mouseMove /= windowSize;

		glm::vec3 forward = glm::normalize(transforms->orientation * glm::vec3(0.0f, 0.0f,-1.0f));

		// Set the pitch and yaw
		if (mouseMove != glm::vec2(0.0)) {
			mLastMouseX = userInput.mouseX;
			mLastMouseY = userInput.mouseY;

			// Multiply the values by the mouse speed
			float yaw = kMouseSpeed * -elapsedTime * static_cast<float>(mouseMove.x);
			float pitch = -kMouseSpeed * elapsedTime * static_cast<float>(mouseMove.y);

			// Clamp the pitch
			float currentPitch = std::asin(forward.y);
			float nextPitch = currentPitch + pitch;
			nextPitch = std::clamp(nextPitch, -glm::half_pi<float>() + kPitchLimit, glm::half_pi<float>() - kPitchLimit);
			pitch = nextPitch - currentPitch;

			SOMBRA_DEBUG_LOG << "Updating the entity " << mEntity << " orientation (" << pitch << ", " << yaw << ")";

			// Apply the rotation
			glm::quat qYaw = glm::angleAxis(yaw, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::quat qPitch = glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));
			transforms->orientation = glm::normalize(qYaw * transforms->orientation * qPitch);
			transforms->updated.reset();

			forward = glm::normalize(transforms->orientation * glm::vec3(0.0f, 0.0f,-1.0f));
		}

		glm::vec3 up = { 0.0f, 1.0f, 0.0f };
		glm::vec3 right = glm::cross(forward, up);

		// Add WASD movement
		glm::vec3 direction(0.0f);
		if (userInput.keys[SE_KEY_W]) { direction += forward; }
		if (userInput.keys[SE_KEY_S]) { direction -= forward; }
		if (userInput.keys[SE_KEY_D]) { direction += right; }
		if (userInput.keys[SE_KEY_A]) { direction -= right; }
		float length = glm::length(direction);
		if (length > 0.0f) {
			transforms->velocity += kRunSpeed * elapsedTime * direction / length;
			SOMBRA_DEBUG_LOG << "Updating the entity " << mEntity << " run velocity (" << glm::to_string(transforms->velocity) << ")";
			transforms->updated.reset();
		}

		// Add the world Y velocity
		direction = glm::vec3(0.0f);
		if (userInput.keys[SE_KEY_SPACE]) { direction += up; }
		if (userInput.keys[SE_KEY_LEFT_CONTROL]) { direction -= up; }
		length = glm::length(direction);
		if (length > 0.0f) {
			transforms->velocity += kJumpSpeed * elapsedTime * direction;
			SOMBRA_DEBUG_LOG << "Updating the entity " << mEntity << " jump velocity (" << glm::to_string(transforms->velocity) << ")";
			transforms->updated.reset();
		}

		// Other
		if (userInput.keys[SE_KEY_P]) {
			PRINT = !PRINT;
		}

		if (userInput.mouseButtons[SE_MOUSE_BUTTON_LEFT]) {
			std::string names;
			auto [collider, rayCast] = mLevel.getGame().getExternalTools().rigidBodyWorld->getCollisionDetector().rayCastFirst(transforms->position + 1.5f * forward, forward);
			if (collider) {
				// Blue tetrahedron = separation direction from collider 1 to collider 0
				glm::vec3 new_z = rayCast.contactNormal;
				glm::vec3 new_x = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), new_z));
				glm::vec3 new_y = glm::normalize(glm::cross(new_z, new_x));

				auto pointEntity = mLevel.getGame().getEntityDatabase().addEntity();
				mLevel.getScene().entities.push_back(pointEntity);

				se::app::TransformsComponent transforms2;
				transforms2.position = rayCast.contactPointWorld;
				transforms2.orientation = glm::quat_cast(glm::mat4(glm::mat3(new_x, new_y, new_z)));
				mLevel.getGame().getEntityDatabase().addComponent(pointEntity, std::move(transforms2));

				se::app::MeshComponent mesh;
				std::size_t iTetraMesh = mesh.add(false, mTetrahedronMesh);
				mesh.addRenderableShader(iTetraMesh, mShaderYellow);
				mLevel.getGame().getEntityDatabase().addComponent(pointEntity, std::move(mesh));

				auto light = mLevel.getGame().getEntityDatabase().emplaceComponent<se::app::LightComponent>(pointEntity);
				light->setSource(mLightYellow);

				se::app::Entity selectedEntity = static_cast<se::app::Entity>(reinterpret_cast<intptr_t>(collider->getParent()->getProperties().userData));
				auto [tag] = mLevel.getGame().getEntityDatabase().getComponents<se::app::TagComponent>(selectedEntity, true);
				names += std::string(tag? tag->getName() : "") + "; ";
			}

			mPickText.setText(glm::to_string(transforms->position) + " " + glm::to_string(forward) + " Selected entities: " + names);
		}
	}

}
