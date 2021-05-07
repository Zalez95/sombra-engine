#include "se/utils/Log.h"
#include "se/graphics/3D/Renderer3D.h"
#include "se/graphics/RenderGraph.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/app/CameraSystem.h"
#include "se/app/Application.h"
#include "se/app/TransformsComponent.h"
#include "se/app/MeshComponent.h"
#include "se/app/TerrainComponent.h"
#include "se/app/ParticleSystemComponent.h"
#include "se/app/IViewProjectionUpdater.h"

namespace se::app {

	class CameraSystem::CameraUniformsUpdater : public IViewProjectionUpdater
	{
	private:
		CameraSystem* mParent;
		std::vector<graphics::Renderer*> mRenderers;

	public:		// Functions
		CameraUniformsUpdater(
			CameraSystem* parent, std::vector<graphics::Renderer*> renderers,
			const char* viewMatUniformName, const char* projectionMatUniformName
		) : IViewProjectionUpdater(viewMatUniformName, projectionMatUniformName),
			mParent(parent), mRenderers(renderers) {};

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

		virtual bool shouldAddUniforms(const PassSPtr& pass) const override
		{
			return std::find(mRenderers.begin(), mRenderers.end(), &pass->getRenderer()) != mRenderers.end();
		};
	};


	CameraSystem::CameraSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application), mCamera(nullptr)
	{
		mApplication.getEventManager()
			.subscribe(this, Topic::Camera)
			.subscribe(this, Topic::RMesh)
			.subscribe(this, Topic::RShader)
			.subscribe(this, Topic::Shader);
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask()
			.set<CameraComponent>()
			.set<MeshComponent>()
			.set<TerrainComponent>()
			.set<ParticleSystemComponent>()
		);

		auto& renderGraph = mApplication.getExternalTools().graphicsEngine->getRenderGraph();
		auto forwardRendererMesh = dynamic_cast<graphics::Renderer3D*>(renderGraph.getNode("forwardRendererMesh"));
		auto gBufferRendererTerrain = dynamic_cast<graphics::Renderer3D*>(renderGraph.getNode("gBufferRendererTerrain"));
		auto gBufferRendererMesh = dynamic_cast<graphics::Renderer3D*>(renderGraph.getNode("gBufferRendererMesh"));
		auto gBufferRendererParticles = dynamic_cast<graphics::Renderer3D*>(renderGraph.getNode("gBufferRendererParticles"));
		mDeferredLightRenderer = dynamic_cast<DeferredLightRenderer*>(renderGraph.getNode("deferredLightRenderer"));

		mCameraUniformsUpdater = new CameraUniformsUpdater(
			this, { forwardRendererMesh, gBufferRendererMesh, gBufferRendererTerrain, gBufferRendererParticles },
			"uViewMatrix", "uProjectionMatrix"
		);

		mFrustumFilter = std::make_shared<graphics::FrustumFilter>();
		forwardRendererMesh->addFilter(mFrustumFilter);
		gBufferRendererTerrain->addFilter(mFrustumFilter);
		gBufferRendererMesh->addFilter(mFrustumFilter);
		gBufferRendererParticles->addFilter(mFrustumFilter);
	}


	CameraSystem::~CameraSystem()
	{
		delete mCameraUniformsUpdater;

		mEntityDatabase.removeSystem(this);
		mApplication.getEventManager()
			.unsubscribe(this, Topic::Shader)
			.unsubscribe(this, Topic::RShader)
			.unsubscribe(this, Topic::RMesh)
			.unsubscribe(this, Topic::Camera);
	}


	void CameraSystem::notify(const IEvent& event)
	{
		tryCall(&CameraSystem::onCameraEvent, event);
		tryCall(&CameraSystem::onRMeshEvent, event);
		tryCall(&CameraSystem::onRenderableShaderEvent, event);
		tryCall(&CameraSystem::onShaderEvent, event);
	}


	void CameraSystem::onNewEntity(Entity entity)
	{
		auto [transforms, camera, mesh, terrain, particleSystem] = mEntityDatabase.getComponents<
			TransformsComponent, CameraComponent, MeshComponent, TerrainComponent, ParticleSystemComponent
		>(entity);

		if (camera) {
			if (transforms) {
				transforms->updated.reset(static_cast<int>(TransformsComponent::Update::Camera));
			}

			SOMBRA_INFO_LOG << "Entity " << entity << " with Camera " << camera << " added successfully";
		}

		if (mesh) {
			mesh->processRenderableIndices([&, mesh = mesh](std::size_t i) {
				mCameraUniformsUpdater->addRenderable(mesh->get(i));
				mesh->processRenderableShaders(i, [&](const auto& shader) {
					mCameraUniformsUpdater->addRenderableShader(mesh->get(i), shader);
				});
			});
		}
		if (terrain) {
			mCameraUniformsUpdater->addRenderable(terrain->get());
			terrain->processRenderableShaders([&](const auto& shader) {
				mCameraUniformsUpdater->addRenderableShader(terrain->get(), shader);
			});
		}
		if (particleSystem) {
			mCameraUniformsUpdater->addRenderable(particleSystem->get());
			particleSystem->processRenderableShaders([&](const auto& shader) {
				mCameraUniformsUpdater->addRenderableShader(particleSystem->get(), shader);
			});
		}
	}


	void CameraSystem::onRemoveEntity(Entity entity)
	{
		auto [camera, mesh, terrain, particleSystem] = mEntityDatabase.getComponents<
			CameraComponent, MeshComponent, TerrainComponent, ParticleSystemComponent
		>(entity);

		if (mesh) {
			mesh->processRenderableIndices([&, mesh = mesh](std::size_t i) {
				mCameraUniformsUpdater->removeRenderable(mesh->get(i));
			});
		}
		if (terrain) {
			mCameraUniformsUpdater->removeRenderable(terrain->get());
		}
		if (particleSystem) {
			mCameraUniformsUpdater->removeRenderable(particleSystem->get());
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
				if (!transforms->updated[static_cast<int>(TransformsComponent::Update::Camera)]) {
					camera->setPosition(transforms->position);
					camera->setOrientation(transforms->orientation);
					transforms->updated.set(static_cast<int>(TransformsComponent::Update::Camera));
				}
			}
		);

		SOMBRA_DEBUG_LOG << "Updating the Uniforms";
		mCameraUniformsUpdater->update();

		if (mCamera) {
			SOMBRA_DEBUG_LOG << "Updating the Renderers";
			glm::mat4 viewProjectionMatrix = mCamera->getProjectionMatrix() * mCamera->getViewMatrix();
			mFrustumFilter->updateFrustum(viewProjectionMatrix);
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


	void CameraSystem::onRMeshEvent(const RMeshEvent& event)
	{
		auto [mesh] = mEntityDatabase.getComponents<MeshComponent>(event.getEntity());
		if (mesh) {
			switch (event.getOperation()) {
				case RMeshEvent::Operation::Add:
					mCameraUniformsUpdater->addRenderable(mesh->get(event.getRIndex()));
					break;
				case RMeshEvent::Operation::Remove:
					mCameraUniformsUpdater->removeRenderable(mesh->get(event.getRIndex()));
					break;
			}
		}
	}


	void CameraSystem::onRenderableShaderEvent(const RenderableShaderEvent& event)
	{
		if (event.getRComponentType() == RenderableShaderEvent::RComponentType::Mesh) {
			auto [mesh] = mEntityDatabase.getComponents<MeshComponent>(event.getEntity());
			if (mesh) {
				switch (event.getOperation()) {
					case RenderableShaderEvent::Operation::Add:
						mCameraUniformsUpdater->addRenderableShader(mesh->get(event.getRIndex()), event.getShader());
						break;
					case RenderableShaderEvent::Operation::Remove:
						mCameraUniformsUpdater->removeRenderableShader(mesh->get(event.getRIndex()), event.getShader());
						break;
				}
			}
		}
		else {
			graphics::Renderable* renderable = nullptr;
			if (event.getRComponentType() == RenderableShaderEvent::RComponentType::Terrain) {
				auto [terrain] = mEntityDatabase.getComponents<TerrainComponent>(event.getEntity());
				renderable = &terrain->get();
			}
			else if (event.getRComponentType() == RenderableShaderEvent::RComponentType::ParticleSystem) {
				auto [particleSystem] = mEntityDatabase.getComponents<ParticleSystemComponent>(event.getEntity());
				renderable = &particleSystem->get();
			}

			if (renderable) {
				switch (event.getOperation()) {
					case RenderableShaderEvent::Operation::Add:
						mCameraUniformsUpdater->addRenderableShader(*renderable, event.getShader());
						break;
					case RenderableShaderEvent::Operation::Remove:
						mCameraUniformsUpdater->removeRenderableShader(*renderable, event.getShader());
						break;
				}
			}
		}
	}


	void CameraSystem::onShaderEvent(const ShaderEvent& event)
	{
		switch (event.getOperation()) {
			case ShaderEvent::Operation::Add:
				mCameraUniformsUpdater->onAddShaderPass(event.getShader(), event.getPass());
				break;
			case ShaderEvent::Operation::Remove:
				mCameraUniformsUpdater->onRemoveShaderPass(event.getShader(), event.getPass());
				break;
		}
	}

}
