#include "se/utils/Log.h"
#include "se/app/CameraSystem.h"
#include "se/app/Application.h"
#include "se/app/EntityDatabase.h"
#include "se/app/TransformsComponent.h"
#include "se/graphics/Renderer.h"
#include "se/graphics/RenderGraph.h"
#include "se/graphics/GraphicsEngine.h"

namespace se::app {

	CameraSystem::CameraSystem(Application& application) :
		IVPSystem(application, "uViewMatrix", "uProjectionMatrix"),
		mCameraEntity(kNullEntity), mCamera(nullptr)
	{
		mApplication.getEventManager().subscribe(this, Topic::Camera);
		mEntityDatabase.addSystem(this, mEntityDatabase.getSystemMask(this).set<CameraComponent>());

		auto& renderGraph = mApplication.getExternalTools().graphicsEngine->getRenderGraph();
		mForwardRenderer = renderGraph.getNode("forwardRenderer");
		mGBufferRenderer = renderGraph.getNode("gBufferRenderer");
	}


	CameraSystem::~CameraSystem()
	{
		mEntityDatabase.removeSystem(this);
		mApplication.getEventManager().unsubscribe(this, Topic::Camera);
	}


	void CameraSystem::notify(const IEvent& event)
	{
		tryCall(&CameraSystem::onCameraEvent, event);
	}


	void CameraSystem::onNewEntity(Entity entity)
	{
		auto [transforms, camera] = mEntityDatabase.getComponents<TransformsComponent, CameraComponent>(entity);
		if (camera) {
			if (transforms) {
				// The Camera initial data is overridden by the entity one
				camera->setPosition(transforms->position);
				camera->setTarget(transforms->position + glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation);
				camera->setUp({ 0.0f, 1.0f, 0.0f });
			}

			SOMBRA_INFO_LOG << "Entity " << entity << " with Camera " << camera << " added successfully";
		}

		IVPSystem::onNewEntity(entity);
	}


	void CameraSystem::onRemoveEntity(Entity entity)
	{
		if (mCameraEntity == entity) {
			mCameraEntity = kNullEntity;
			mCamera = nullptr;
			SOMBRA_INFO_LOG << "Active Camera Entity " << entity << " removed";
		}

		IVPSystem::onRemoveEntity(entity);

		SOMBRA_INFO_LOG << "Entity " << entity << " removed successfully";
	}


	void CameraSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Cameras";

		mEntityDatabase.iterateComponents<TransformsComponent, CameraComponent>(
			[&](Entity, TransformsComponent* transforms, CameraComponent* camera) {
				if (transforms->updated.any()) {
					camera->setPosition(transforms->position);
					camera->setTarget(transforms->position + glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation);
					camera->setUp({ 0.0f, 1.0f, 0.0f });
				}
			}
		);

		IVPSystem::update();

		SOMBRA_INFO_LOG << "Update end";
	}

// Private functions
	glm::mat4 CameraSystem::getViewMatrix() const
	{
		if (mCamera) {
			return mCamera->getViewMatrix();
		}
		return glm::mat4(1.0f);
	}


	glm::mat4 CameraSystem::getProjectionMatrix() const
	{
		if (mCamera) {
			return mCamera->getProjectionMatrix();
		}
		return glm::mat4(1.0f);
	}


	bool CameraSystem::shouldAddUniforms(PassSPtr pass) const
	{
		return (&pass->getRenderer() == mForwardRenderer) || (&pass->getRenderer() == mGBufferRenderer);
	}


	void CameraSystem::onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event)
	{
		mCameraEntity = event.getValue();
		mCamera = std::get<0>( mEntityDatabase.getComponents<CameraComponent>(mCameraEntity) );
	}

}
