#include "se/utils/Log.h"
#include "se/graphics/Renderer.h"
#include "se/graphics/RenderGraph.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/graphics/3D/RenderableTerrain.h"
#include "se/app/CameraSystem.h"
#include "se/app/Application.h"
#include "se/app/EntityDatabase.h"
#include "se/app/TransformsComponent.h"
#include "se/app/MeshComponent.h"
#include "se/app/graphics/IViewProjectionUpdater.h"

namespace se::app {

	class CameraSystem::CameraUniformsUpdater : public IViewProjectionUpdater
	{
	private:
		CameraSystem* mParent;

	public:		// Functions
		CameraUniformsUpdater(CameraSystem* parent,
			const std::string& viewMatUniformName,
			const std::string& projectionMatUniformName
		) : IViewProjectionUpdater(viewMatUniformName, projectionMatUniformName),
			mParent(parent) {};

		virtual glm::mat4 getViewMatrix() const override
		{
			if (mParent->mCamera) {
				return mParent->mCamera->getViewMatrix();
			}
			return glm::mat4(1.0f);
		};

		virtual glm::mat4 getProjectionMatrix() const override
		{
			if (mParent->mCamera) {
				return mParent->mCamera->getProjectionMatrix();
			}
			return glm::mat4(1.0f);
		};

		virtual bool shouldAddUniforms(PassSPtr pass) const override
		{
			return (&pass->getRenderer() == mParent->mForwardRenderer)
				|| (&pass->getRenderer() == mParent->mGBufferRenderer);
		};
	};


	CameraSystem::CameraSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application), mCamera(nullptr)
	{
		mApplication.getEventManager().subscribe(this, Topic::Camera);
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask()
			.set<CameraComponent>()
			.set<MeshComponent>()
			.set<graphics::RenderableTerrain>()
		);

		mCameraUniformsUpdater = new CameraUniformsUpdater(this, "uViewMatrix", "uProjectionMatrix");

		auto& renderGraph = mApplication.getExternalTools().graphicsEngine->getRenderGraph();
		mForwardRenderer = dynamic_cast<FrustumRenderer3D*>(renderGraph.getNode("forwardRenderer"));
		mGBufferRenderer = dynamic_cast<FrustumRenderer3D*>(renderGraph.getNode("gBufferRenderer"));
		mDeferredLightRenderer = dynamic_cast<DeferredLightRenderer*>(renderGraph.getNode("deferredLightRenderer"));
	}


	CameraSystem::~CameraSystem()
	{
		delete mCameraUniformsUpdater;

		mEntityDatabase.removeSystem(this);
		mApplication.getEventManager().unsubscribe(this, Topic::Camera);
	}


	void CameraSystem::notify(const IEvent& event)
	{
		tryCall(&CameraSystem::onCameraEvent, event);
	}


	void CameraSystem::onNewEntity(Entity entity)
	{
		auto [transforms, camera, mesh, rTerrain] = mEntityDatabase.getComponents<
			TransformsComponent, CameraComponent, MeshComponent, graphics::RenderableTerrain
		>(entity);

		if (camera) {
			if (transforms) {
				// The Camera initial data is overridden by the entity one
				camera->setPosition(transforms->position);
				camera->setTarget(transforms->position + glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation);
				camera->setUp({ 0.0f, 1.0f, 0.0f });
			}

			SOMBRA_INFO_LOG << "Entity " << entity << " with Camera " << camera << " added successfully";
		}

		if (mesh) {
			for (auto& rMesh : mesh->rMeshes) {
				mCameraUniformsUpdater->addRenderable(rMesh);
			}
		}
		if (rTerrain) {
			mCameraUniformsUpdater->addRenderable(*rTerrain);
		}
	}


	void CameraSystem::onRemoveEntity(Entity entity)
	{
		auto [camera, mesh, rTerrain] = mEntityDatabase.getComponents<
			CameraComponent, MeshComponent, graphics::RenderableTerrain
		>(entity);

		if (mesh) {
			for (auto& rMesh : mesh->rMeshes) {
				mCameraUniformsUpdater->removeRenderable(rMesh);
			}
		}
		if (rTerrain) {
			mCameraUniformsUpdater->removeRenderable(*rTerrain);
		}

		if (mCamera == camera) {
			mCamera = nullptr;
			SOMBRA_INFO_LOG << "Active Camera removed";
		}

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

		SOMBRA_DEBUG_LOG << "Updating the Uniforms";
		mCameraUniformsUpdater->update();

		if (mCamera) {
			SOMBRA_DEBUG_LOG << "Updating the Renderers";
			glm::mat4 viewProjectionMatrix = mCamera->getProjectionMatrix() * mCamera->getViewMatrix();
			mForwardRenderer->updateFrustum(viewProjectionMatrix);
			mGBufferRenderer->updateFrustum(viewProjectionMatrix);
			mDeferredLightRenderer->setViewPosition(mCamera->getPosition());
		}

		SOMBRA_INFO_LOG << "Update end";
	}

// Private functions
	void CameraSystem::onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event)
	{
		auto [camera] = mEntityDatabase.getComponents<CameraComponent>(event.getValue());
		if (camera) {
			mCamera = camera;
		}
		else {
			SOMBRA_WARN_LOG << "Couldn't set Entity " << event.getValue() << " as Camera Entity";
		}
	}

}
