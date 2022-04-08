#include "se/utils/Log.h"
#include "se/graphics/3D/Renderer3D.h"
#include "se/graphics/RenderGraph.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/app/CameraSystem.h"
#include "se/app/Application.h"
#include "se/app/CameraComponent.h"
#include "se/app/TransformsComponent.h"
#include "se/app/MeshComponent.h"
#include "se/app/TerrainComponent.h"
#include "se/app/ParticleSystemComponent.h"
#include "se/app/LightComponent.h"
#include "graphics/SSAONode.h"
#include "graphics/DeferredLightSubGraph.h"
#include "graphics/DeferredAmbientRenderer.h"
#include "graphics/IViewProjectionUpdater.h"

namespace se::app {

	class CameraSystem::CameraUniformsUpdater : public IViewProjectionUpdater
	{
	private:
		std::vector<graphics::Renderer*> mRenderers;

	public:		// Functions
		CameraUniformsUpdater(
			std::vector<graphics::Renderer*> renderers, graphics::Context& context,
			const char* viewMatUniformName, const char* projectionMatUniformName
		) : IViewProjectionUpdater(context, viewMatUniformName, projectionMatUniformName),
			mRenderers(renderers) {};

		virtual bool shouldAddUniforms(const PassSPtr& pass) const override
		{
			return std::find(mRenderers.begin(), mRenderers.end(), &pass->getRenderer()) != mRenderers.end();
		};
	};


	CameraSystem::CameraSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application), mCameraEntity(kNullEntity),
		mCameraUniformsUpdater(nullptr), mDeferredAmbientRenderer(nullptr), mSSAONode(nullptr)
	{
		mApplication.getEventManager()
			.subscribe(this, Topic::Camera)
			.subscribe(this, Topic::RMesh)
			.subscribe(this, Topic::RShader)
			.subscribe(this, Topic::Shader);
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask()
			.set<CameraComponent>()
			.set<TransformsComponent>()
			.set<MeshComponent>()
			.set<TerrainComponent>()
			.set<ParticleSystemComponent>()
			.set<LightComponent>()
		);

		auto& context = mApplication.getExternalTools().graphicsEngine->getContext();
		const auto& renderGraph = mApplication.getExternalTools().graphicsEngine->getRenderGraph();
		auto gBufferRendererMesh = dynamic_cast<graphics::Renderer3D*>(renderGraph.getNode("gBufferRendererMesh"));
		auto gBufferRendererTerrain = dynamic_cast<graphics::Renderer3D*>(renderGraph.getNode("gBufferRendererTerrain"));
		auto gBufferRendererParticles = dynamic_cast<graphics::Renderer3D*>(renderGraph.getNode("gBufferRendererParticles"));
		auto deferredLightSubGraph = dynamic_cast<DeferredLightSubGraph*>(renderGraph.getNode("deferredLightSubGraph"));
		auto forwardRendererMesh = dynamic_cast<graphics::Renderer3D*>(renderGraph.getNode("forwardRendererMesh"));
		mDeferredAmbientRenderer = dynamic_cast<DeferredAmbientRenderer*>(renderGraph.getNode("deferredAmbientRenderer"));
		mSSAONode = dynamic_cast<SSAONode*>(renderGraph.getNode("ssaoNode"));
		auto deferredLightStencilRenderer = deferredLightSubGraph->getStencilRenderer();
		auto deferredLightColorRenderer = deferredLightSubGraph->getColorRenderer();

		mCameraUniformsUpdater = new CameraUniformsUpdater(
			{ gBufferRendererMesh, gBufferRendererTerrain, gBufferRendererParticles, deferredLightStencilRenderer, deferredLightColorRenderer, forwardRendererMesh },
			context, "uViewMatrix", "uProjectionMatrix"
		);

		mFrustumFilter = std::make_shared<graphics::FrustumFilter>();
		deferredLightSubGraph->addFilter(mFrustumFilter);
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


	bool CameraSystem::notify(const IEvent& event)
	{
		return tryCall(&CameraSystem::onCameraEvent, event)
			|| tryCall(&CameraSystem::onRMeshEvent, event)
			|| tryCall(&CameraSystem::onRenderableShaderEvent, event)
			|| tryCall(&CameraSystem::onShaderEvent, event);
	}


	void CameraSystem::onNewComponent(Entity entity, const EntityDatabase::ComponentMask& mask, EntityDatabase::Query& query)
	{
		tryCallC(&CameraSystem::onNewCamera, entity, mask, query);
		tryCallC(&CameraSystem::onNewTransforms, entity, mask, query);
		tryCallC(&CameraSystem::onNewMesh, entity, mask, query);
		tryCallC(&CameraSystem::onNewTerrain, entity, mask, query);
		tryCallC(&CameraSystem::onNewParticleSys, entity, mask, query);
		tryCallC(&CameraSystem::onNewLight, entity, mask, query);
	}


	void CameraSystem::onRemoveComponent(Entity entity, const EntityDatabase::ComponentMask& mask, EntityDatabase::Query& query)
	{
		tryCallC(&CameraSystem::onRemoveCamera, entity, mask, query);
		tryCallC(&CameraSystem::onRemoveMesh, entity, mask, query);
		tryCallC(&CameraSystem::onRemoveTerrain, entity, mask, query);
		tryCallC(&CameraSystem::onRemoveParticleSys, entity, mask, query);
		tryCallC(&CameraSystem::onRemoveLight, entity, mask, query);
	}


	void CameraSystem::update(float, float)
	{
		SOMBRA_DEBUG_LOG << "Start";

		glm::vec3 viewPosition;
		glm::mat4 viewMatrix, projectionMatrix, viewProjectionMatrix;
		mEntityDatabase.executeQuery([&](EntityDatabase::Query& query) {
			SOMBRA_DEBUG_LOG << "Updating the Cameras";
			query.iterateEntityComponents<TransformsComponent, CameraComponent>(
				[&](Entity, TransformsComponent* transforms, CameraComponent* camera) {
					if (!transforms->updated[static_cast<int>(TransformsComponent::Update::Camera)]) {
						camera->setPosition(transforms->position);
						camera->setOrientation(transforms->orientation);
						transforms->updated.set(static_cast<int>(TransformsComponent::Update::Camera));
					}
				},
				true
			);

			auto [transforms, camera] = query.getComponents<TransformsComponent, CameraComponent>(mCameraEntity, true);
			viewPosition = (transforms)? transforms->position : glm::vec3(0.0f);
			viewMatrix = (camera)? camera->getViewMatrix() : glm::mat4(1.0f);
			projectionMatrix = (camera)? camera->getProjectionMatrix() : glm::mat4(1.0f);
			viewProjectionMatrix = projectionMatrix * viewMatrix;
		});

		SOMBRA_DEBUG_LOG << "Updating the Uniforms";
		mCameraUniformsUpdater->updateUniformsDeferred(viewMatrix, projectionMatrix);

		SOMBRA_DEBUG_LOG << "Updating the Renderers";
		mDeferredAmbientRenderer->setViewPosition(viewPosition);
		mSSAONode->setViewMatrix(viewMatrix);
		mSSAONode->setProjectionMatrix(projectionMatrix);
		mFrustumFilter->updateFrustum(viewProjectionMatrix);

		SOMBRA_DEBUG_LOG << "Update end";
	}

// Private functions
	void CameraSystem::onNewCamera(Entity entity, CameraComponent* camera, EntityDatabase::Query& query)
	{
		auto [transforms] = query.getComponents<TransformsComponent>(entity, true);
		if (transforms) {
			transforms->updated.reset(static_cast<int>(TransformsComponent::Update::Camera));
		}

		SOMBRA_INFO_LOG << "Entity " << entity << " with CameraComponent " << camera << " added successfully";
	}


	void CameraSystem::onRemoveCamera(Entity entity, CameraComponent* camera, EntityDatabase::Query&)
	{
		if (mCameraEntity == entity) {
			mCameraEntity = kNullEntity;
			SOMBRA_INFO_LOG << "Active Camera removed";
		}

		SOMBRA_INFO_LOG << "Entity " << entity << " with CameraComponent " << camera << " removed successfully";
	}


	void CameraSystem::onNewTransforms(Entity, TransformsComponent* transforms, EntityDatabase::Query&)
	{
		transforms->updated.reset(static_cast<int>(TransformsComponent::Update::Camera));
	}


	void CameraSystem::onNewMesh(Entity entity, MeshComponent* mesh, EntityDatabase::Query&)
	{
		mesh->processRenderableIndices([&, mesh = mesh](std::size_t i) {
			mesh->processRenderableShaders(i, [&](const auto& shader) {
				mCameraUniformsUpdater->addRenderableTechnique(mesh->get(i), shader->getTechnique());
			});
		});
		SOMBRA_INFO_LOG << "Entity " << entity << " with MeshComponent " << mesh << " added successfully";
	}


	void CameraSystem::onRemoveMesh(Entity entity, MeshComponent* mesh, EntityDatabase::Query&)
	{
		mesh->processRenderableIndices([&, mesh = mesh](std::size_t i) {
			mesh->processRenderableShaders(i, [&](const auto& shader) {
				mCameraUniformsUpdater->removeRenderableTechnique(mesh->get(i), shader->getTechnique());
			});
		});
		SOMBRA_INFO_LOG << "Entity " << entity << " with MeshComponent " << mesh << " removed successfully";
	}


	void CameraSystem::onNewTerrain(Entity entity, TerrainComponent* terrain, EntityDatabase::Query&)
	{
		terrain->processRenderableShaders([&](const auto& shader) {
			mCameraUniformsUpdater->addRenderableTechnique(terrain->get(), shader->getTechnique());
		});
		SOMBRA_INFO_LOG << "Entity " << entity << " with TerrainComponent " << terrain << " added successfully";
	}


	void CameraSystem::onRemoveTerrain(Entity entity, TerrainComponent* terrain, EntityDatabase::Query&)
	{
		terrain->processRenderableShaders([&](const auto& shader) {
			mCameraUniformsUpdater->removeRenderableTechnique(terrain->get(), shader->getTechnique());
		});
		SOMBRA_INFO_LOG << "Entity " << entity << " with TerrainComponent " << terrain << " removed successfully";
	}


	void CameraSystem::onNewParticleSys(Entity entity, ParticleSystemComponent* particleSystem, EntityDatabase::Query&)
	{
		particleSystem->processRenderableShaders([&](const auto& shader) {
			mCameraUniformsUpdater->addRenderableTechnique(particleSystem->get(), shader->getTechnique());
		});
		SOMBRA_INFO_LOG << "Entity " << entity << " with ParticleSystemComponent " << particleSystem << " added successfully";
	}


	void CameraSystem::onRemoveParticleSys(Entity entity, ParticleSystemComponent* particleSystem, EntityDatabase::Query&)
	{
		particleSystem->processRenderableShaders([&](const auto& shader) {
			mCameraUniformsUpdater->removeRenderableTechnique(particleSystem->get(), shader->getTechnique());
		});
		SOMBRA_INFO_LOG << "Entity " << entity << " with ParticleSystemComponent " << particleSystem << " removed successfully";
	}


	void CameraSystem::onNewLight(Entity entity, LightComponent* light, EntityDatabase::Query&)
	{
		auto renderable = light->getRenderable().getRenderableMesh();
		renderable.processTechniques([&](const auto& technique) {
			mCameraUniformsUpdater->addRenderableTechnique(renderable, technique);
		});
		SOMBRA_INFO_LOG << "Entity " << entity << " with LightComponent " << light << " added successfully";
	}


	void CameraSystem::onRemoveLight(Entity entity, LightComponent* light, EntityDatabase::Query&)
	{
		auto renderable = light->getRenderable().getRenderableMesh();
		renderable.processTechniques([&](const auto& technique) {
			mCameraUniformsUpdater->removeRenderableTechnique(renderable, technique);
		});
		SOMBRA_INFO_LOG << "Entity " << entity << " with LightComponent " << light << " removed successfully";
	}


	void CameraSystem::onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event)
	{
		SOMBRA_INFO_LOG << event;

		mCameraEntity = event.getValue();
		SOMBRA_INFO_LOG << "Entity " << mCameraEntity << " setted as camera";
	}


	void CameraSystem::onRMeshEvent(const RMeshEvent& event)
	{
		SOMBRA_INFO_LOG << event;

		mEntityDatabase.executeQuery([&](EntityDatabase::Query& query) {
			auto [mesh] = query.getComponents<MeshComponent>(event.getEntity(), true);
			if (mesh) {
				switch (event.getOperation()) {
					case RMeshEvent::Operation::Add:
						mesh->processRenderableShaders(event.getRIndex(), [&](const auto& shader) {
							mCameraUniformsUpdater->addRenderableTechnique(mesh->get(event.getRIndex()), shader->getTechnique());
						});
						break;
					case RMeshEvent::Operation::Remove:
						mesh->processRenderableShaders(event.getRIndex(), [&](const auto& shader) {
							mCameraUniformsUpdater->removeRenderableTechnique(mesh->get(event.getRIndex()), shader->getTechnique());
						});
						break;
				}
			}
		});
	}


	void CameraSystem::onRenderableShaderEvent(const RenderableShaderEvent& event)
	{
		SOMBRA_INFO_LOG << event;

		mEntityDatabase.executeQuery([&](EntityDatabase::Query& query) {
			if (event.getRComponentType() == RenderableShaderEvent::RComponentType::Mesh) {
				auto [mesh] = query.getComponents<MeshComponent>(event.getEntity(), true);
				if (mesh) {
					switch (event.getOperation()) {
						case RenderableShaderEvent::Operation::Add:
							mCameraUniformsUpdater->addRenderableTechnique(mesh->get(event.getRIndex()), event.getShader()->getTechnique());
							break;
						case RenderableShaderEvent::Operation::Remove:
							mCameraUniformsUpdater->removeRenderableTechnique(mesh->get(event.getRIndex()), event.getShader()->getTechnique());
							break;
					}
				}
			}
			else {
				graphics::Renderable* renderable = nullptr;
				if (event.getRComponentType() == RenderableShaderEvent::RComponentType::Terrain) {
					auto [terrain] = query.getComponents<TerrainComponent>(event.getEntity(), true);
					renderable = &terrain->get();
				}
				else if (event.getRComponentType() == RenderableShaderEvent::RComponentType::ParticleSystem) {
					auto [particleSystem] = query.getComponents<ParticleSystemComponent>(event.getEntity(), true);
					renderable = &particleSystem->get();
				}
				else if (event.getRComponentType() == RenderableShaderEvent::RComponentType::Light) {
					auto [light] = query.getComponents<LightComponent>(event.getEntity(), true);
					renderable = &light->getRenderable().getRenderableMesh();
				}

				if (renderable) {
					switch (event.getOperation()) {
						case RenderableShaderEvent::Operation::Add:
							mCameraUniformsUpdater->addRenderableTechnique(*renderable, event.getShader()->getTechnique());
							break;
						case RenderableShaderEvent::Operation::Remove:
							mCameraUniformsUpdater->removeRenderableTechnique(*renderable, event.getShader()->getTechnique());
							break;
					}
				}
			}
		});
	}


	void CameraSystem::onShaderEvent(const ShaderEvent& event)
	{
		SOMBRA_INFO_LOG << event;

		switch (event.getOperation()) {
			case ShaderEvent::Operation::Add:
				mCameraUniformsUpdater->onAddTechniquePass(event.getShader()->getTechnique(), event.getStep()->getPass());
				break;
			case ShaderEvent::Operation::Remove:
				mCameraUniformsUpdater->onRemoveTechniquePass(event.getShader()->getTechnique(), event.getStep()->getPass());
				break;
		}
	}

}
