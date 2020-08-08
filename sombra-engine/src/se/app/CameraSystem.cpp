#include "se/utils/Log.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/graphics/2D/Renderer2D.h"
#include "se/app/CameraSystem.h"
#include "se/app/EntityDatabase.h"
#include "se/app/TransformsComponent.h"
#include "se/app/graphics/Camera.h"
#include "se/app/events/ResizeEvent.h"

namespace se::app {

	CameraSystem::CameraSystem(EntityDatabase& entityDatabase, std::size_t width, std::size_t height) :
		ISystem(entityDatabase), mWidth(width), mHeight(height),
		mActiveCamera(nullptr), mActiveCameraUpdated(false)
	{
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask().set<Camera>());
	}


	CameraSystem::~CameraSystem()
	{
		mEntityDatabase.removeSystem(this);
	}


	CameraSystem::PassSPtr CameraSystem::createPass2D(graphics::Renderer* renderer, ProgramSPtr program)
	{
		auto& passData = mPassesData.emplace_back(true);
		passData.pass = std::make_shared<graphics::Pass>(*renderer);
		passData.program = program;
		passData.projectionMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>(
			"uProjectionMatrix", *program,
			glm::ortho(0.0f, static_cast<float>(mWidth), static_cast<float>(mHeight), 0.0f, -1.0f, 1.0f)
		);

		passData.pass->addBindable(program)
			.addBindable(std::make_shared<graphics::BlendingOperation>(true))
			.addBindable(std::make_shared<graphics::DepthTestOperation>(false))
			.addBindable(passData.projectionMatrix);
		for (int i = 0; i < static_cast<int>(graphics::Renderer2D::kMaxTextures); ++i) {
			utils::ArrayStreambuf<char, 64> aStreambuf;
			std::ostream(&aStreambuf) << "uTextures[" << i << "]";
			passData.pass->addBindable(std::make_shared<graphics::UniformVariableValue<int>>(aStreambuf.data(), *program, i));
		}

		return passData.pass;
	}


	CameraSystem::PassSPtr CameraSystem::createPass3D(graphics::Renderer* renderer, ProgramSPtr program)
	{
		auto& passData = mPassesData.emplace_back(false);
		passData.pass = std::make_shared<graphics::Pass>(*renderer);
		passData.program = program;
		passData.viewMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", *program);
		passData.projectionMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", *program);
		if (mActiveCamera) {
			passData.viewMatrix->setValue(mActiveCamera->getViewMatrix());
			passData.projectionMatrix->setValue(mActiveCamera->getProjectionMatrix());
		}

		passData.pass->addBindable(program)
			.addBindable(passData.viewMatrix)
			.addBindable(passData.projectionMatrix);

		return passData.pass;
	}


	void CameraSystem::notify(const IEvent& event)
	{
		tryCall(&CameraSystem::onResizeEvent, event);
	}


	void CameraSystem::onNewEntity(Entity entity)
	{
		auto [transforms, camera] = mEntityDatabase.getComponents<TransformsComponent, Camera>(entity);
		if (!camera) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as Camera";
			return;
		}

		if (transforms) {
			// The Camera initial data is overridden by the entity one
			camera->setPosition(transforms->position);
			camera->setTarget(transforms->position + glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation);
			camera->setUp({ 0.0f, 1.0f, 0.0f });
		}

		// Add the Camera
		if (!mActiveCamera) {
			mActiveCamera = camera;
		}

		SOMBRA_INFO_LOG << "Entity " << entity << " with Camera " << camera << " added successfully";
	}


	void CameraSystem::onRemoveEntity(Entity entity)
	{
		auto [camera] = mEntityDatabase.getComponents<Camera>(entity);
		if (camera == mActiveCamera) {
			mActiveCamera = nullptr;
		}

		SOMBRA_INFO_LOG << "Camera Entity " << entity << " removed successfully";
	}


	void CameraSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Cameras";

		mActiveCameraUpdated = false;
		mEntityDatabase.iterateComponents<TransformsComponent, Camera>(
			[&](Entity, TransformsComponent* transforms, Camera* camera) {
				if (transforms->updated.any()) {
					camera->setPosition(transforms->position);
					camera->setTarget(transforms->position + glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation);
					camera->setUp({ 0.0f, 1.0f, 0.0f });

					if (camera == mActiveCamera) {
						mActiveCameraUpdated = true;
					}
				}
			}
		);

		if (mActiveCameraUpdated) {
			for (auto& passData : mPassesData) if (!passData.is2D) {
				passData.viewMatrix->setValue(mActiveCamera->getViewMatrix());
				passData.projectionMatrix->setValue(mActiveCamera->getProjectionMatrix());
			}
		}

		SOMBRA_INFO_LOG << "Update end";
	}

// Private functions
	void CameraSystem::onResizeEvent(const ResizeEvent& event)
	{
		mWidth = static_cast<std::size_t>(event.getWidth());
		mHeight = static_cast<std::size_t>(event.getHeight());

		graphics::GraphicsOperations::setViewport(0, 0, mWidth, mHeight);
		for (auto& passData : mPassesData) if (passData.is2D) {
			passData.projectionMatrix->setValue(
				glm::ortho(0.0f, static_cast<float>(mWidth), static_cast<float>(mHeight), 0.0f, -1.0f, 1.0f)
			);
		}
	}

}
