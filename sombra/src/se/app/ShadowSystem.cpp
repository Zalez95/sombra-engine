#include "se/utils/Log.h"
#include "se/graphics/3D/Renderer3D.h"
#include "se/graphics/3D/FrustumFilter.h"
#include "se/graphics/RenderGraph.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/graphics/ViewportResolutionNode.h"
#include "se/app/ShadowSystem.h"
#include "se/app/Application.h"
#include "se/app/TransformsComponent.h"
#include "se/app/MeshComponent.h"
#include "se/app/TerrainComponent.h"
#include "se/app/CameraComponent.h"
#include "se/app/graphics/IViewProjectionUpdater.h"

namespace se::app {

	struct ShadowSystem::Shadow
	{
		ShadowData data;
		CameraComponent camera;
		std::vector<graphics::Renderer*> renderers;
		std::shared_ptr<graphics::FrustumFilter> frustum;
		ShadowUniformsUpdater* uniformUpdater;
	};


	class ShadowSystem::ShadowUniformsUpdater : public IViewProjectionUpdater
	{
	private:
		ShadowSystem* mParent;
		std::size_t mShadowIndex;

	public:		// Functions
		ShadowUniformsUpdater(
			ShadowSystem* parent, std::size_t shadowIndex,
			const char* viewMatUniformName, const char* projectionMatUniformName
		) : IViewProjectionUpdater(viewMatUniformName, projectionMatUniformName),
			mParent(parent), mShadowIndex(shadowIndex) {};

		virtual glm::mat4 getViewMatrix() const override
		{
			return mParent->mShadows[mShadowIndex].camera.getViewMatrix();
		};

		virtual glm::mat4 getProjectionMatrix() const override
		{
			return mParent->mShadows[mShadowIndex].camera.getProjectionMatrix();
		};

		virtual bool shouldAddUniforms(const RenderableShaderStepSPtr& step) const override
		{
			return std::find(
					mParent->mShadows[mShadowIndex].renderers.begin(), mParent->mShadows[mShadowIndex].renderers.end(),
					&step->getPass()->getRenderer()
				) != mParent->mShadows[mShadowIndex].renderers.end();
		};
	};


	ShadowSystem::ShadowSystem(Application& application, const ShadowData& shadowData) :
		ISystem(application.getEntityDatabase()), mApplication(application), mShadowEntity(kNullEntity)
	{
		mApplication.getEventManager()
			.subscribe(this, Topic::Shadow)
			.subscribe(this, Topic::RMesh)
			.subscribe(this, Topic::RShader)
			.subscribe(this, Topic::Shader);
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask()
			.set<LightComponent>()
			.set<MeshComponent>()
			.set<TerrainComponent>()
		);

		auto& renderGraph = mApplication.getExternalTools().graphicsEngine->getRenderGraph();
		auto startShadow = dynamic_cast<graphics::ViewportResolutionNode*>(renderGraph.getNode("startShadow"));
		auto shadowRendererTerrain = dynamic_cast<graphics::Renderer3D*>(renderGraph.getNode("shadowRendererTerrain"));
		auto shadowRendererMesh = dynamic_cast<graphics::Renderer3D*>(renderGraph.getNode("shadowRendererMesh"));
		mDeferredLightRenderer = dynamic_cast<DeferredLightRenderer*>(renderGraph.getNode("deferredLightRenderer"));

		auto& shadow = mShadows.emplace_back();
		shadow.data = shadowData;
		shadow.renderers = { shadowRendererMesh, shadowRendererTerrain };
		shadow.frustum = std::make_shared<graphics::FrustumFilter>();
		shadow.uniformUpdater = new ShadowUniformsUpdater(this, 0, "uViewMatrix", "uProjectionMatrix");

		startShadow->setViewportSize(0, 0, shadow.data.resolution, shadow.data.resolution);
		shadowRendererTerrain->addFilter(shadow.frustum);
		shadowRendererMesh->addFilter(shadow.frustum);
	}


	ShadowSystem::~ShadowSystem()
	{
		for (auto& shadow : mShadows) {
			delete shadow.uniformUpdater;
		}

		mEntityDatabase.removeSystem(this);
		mApplication.getEventManager()
			.unsubscribe(this, Topic::Shader)
			.unsubscribe(this, Topic::RShader)
			.unsubscribe(this, Topic::RMesh)
			.unsubscribe(this, Topic::Shadow);
	}


	void ShadowSystem::notify(const IEvent& event)
	{
		tryCall(&ShadowSystem::onShadowEvent, event);
		tryCall(&ShadowSystem::onRMeshEvent, event);
		tryCall(&ShadowSystem::onRenderableShaderEvent, event);
		tryCall(&ShadowSystem::onShaderEvent, event);
	}


	void ShadowSystem::onNewComponent(Entity entity, const EntityDatabase::ComponentMask& mask)
	{
		tryCallC(&ShadowSystem::onNewLight, entity, mask);
		tryCallC(&ShadowSystem::onNewMesh, entity, mask);
		tryCallC(&ShadowSystem::onNewTerrain, entity, mask);
	}


	void ShadowSystem::onRemoveComponent(Entity entity, const EntityDatabase::ComponentMask& mask)
	{
		tryCallC(&ShadowSystem::onRemoveLight, entity, mask);
		tryCallC(&ShadowSystem::onRemoveMesh, entity, mask);
		tryCallC(&ShadowSystem::onRemoveTerrain, entity, mask);
	}


	void ShadowSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Cameras";

		auto [transforms] = mEntityDatabase.getComponents<TransformsComponent>(mShadowEntity, true);
		if (transforms && !transforms->updated[static_cast<int>(TransformsComponent::Update::Shadow)]) {
			mShadows[0].camera.setPosition(transforms->position);
			mShadows[0].camera.setOrientation(transforms->orientation);
			transforms->updated.set(static_cast<int>(TransformsComponent::Update::Shadow));
		}

		SOMBRA_DEBUG_LOG << "Updating the Uniforms";
		mShadows[0].uniformUpdater->update();

		SOMBRA_DEBUG_LOG << "Updating the Renderers";
		glm::mat4 viewProjectionMatrix = mShadows[0].camera.getProjectionMatrix() * mShadows[0].camera.getViewMatrix();
		mShadows[0].frustum->updateFrustum(viewProjectionMatrix);
		mDeferredLightRenderer->setShadowViewProjectionMatrix(viewProjectionMatrix);

		SOMBRA_INFO_LOG << "Update end";
	}

// Private functions
	void ShadowSystem::onNewLight(Entity entity, LightComponent* light)
	{
		SOMBRA_INFO_LOG << "Entity " << entity << " with LightComponent " << light << " added successfully";
	}


	void ShadowSystem::onRemoveLight(Entity entity, LightComponent* light)
	{
		if (mShadowEntity == entity) {
			mShadowEntity = kNullEntity;
			SOMBRA_INFO_LOG << "Active Shadow Camera removed";
		}

		SOMBRA_INFO_LOG << "Entity " << entity << " with LightComponent " << light << " removed successfully";
	}


	void ShadowSystem::onNewMesh(Entity entity, MeshComponent* mesh)
	{
		mesh->processRenderableIndices([&, mesh = mesh](std::size_t i) {
			for (auto& shadow : mShadows) {
				shadow.uniformUpdater->addRenderable(mesh->get(i));
				mesh->processRenderableShaders(i, [&](const auto& shader) {
					shadow.uniformUpdater->addRenderableShader(mesh->get(i), shader.get());
				});
			}
		});
		SOMBRA_INFO_LOG << "Entity " << entity << " with MeshComponent " << mesh << " added successfully";
	}


	void ShadowSystem::onRemoveMesh(Entity entity, MeshComponent* mesh)
	{
		mesh->processRenderableIndices([&, mesh = mesh](std::size_t i) {
			for (auto& shadow : mShadows) {
				shadow.uniformUpdater->removeRenderable(mesh->get(i));
			}
		});
		SOMBRA_INFO_LOG << "Entity " << entity << " with MeshComponent " << mesh << " removed successfully";
	}


	void ShadowSystem::onNewTerrain(Entity entity, TerrainComponent* terrain)
	{
		for (auto& shadow : mShadows) {
			shadow.uniformUpdater->addRenderable(terrain->get());
			terrain->processRenderableShaders([&](const auto& shader) {
				shadow.uniformUpdater->addRenderableShader(terrain->get(), shader.get());
			});
		}
		SOMBRA_INFO_LOG << "Entity " << entity << " with TerrainComponent " << terrain << " added successfully";
	}


	void ShadowSystem::onRemoveTerrain(Entity entity, TerrainComponent* terrain)
	{
		for (auto& shadow : mShadows) {
			shadow.uniformUpdater->removeRenderable(terrain->get());
		}
		SOMBRA_INFO_LOG << "Entity " << entity << " with TerrainComponent " << terrain << " removed successfully";
	}


	void ShadowSystem::onShadowEvent(const ContainerEvent<Topic::Shadow, Entity>& event)
	{
		auto [transforms, light] = mEntityDatabase.getComponents<TransformsComponent, LightComponent>(event.getValue(), true);
		if (transforms && light && light->source) {
			mShadowEntity = event.getValue();

			transforms->updated.reset(static_cast<int>(TransformsComponent::Update::Shadow));

			if (light->source->type == LightSource::Type::Directional) {
				mShadows[0].camera.setOrthographicProjection(
					-mShadows[0].data.size, mShadows[0].data.size, -mShadows[0].data.size, mShadows[0].data.size,
					mShadows[0].data.zNear, mShadows[0].data.zFar
				);
			}
			else if (light->source->type == LightSource::Type::Spot) {
				mShadows[0].camera.setPerspectiveProjection(
					glm::radians(45.0f), 1.0f,
					mShadows[0].data.zNear, mShadows[0].data.zFar
				);
			}
		}
		else {
			SOMBRA_WARN_LOG << "Couldn't set Entity " << event.getValue() << " as Shadow Entity";
		}
	}


	void ShadowSystem::onRMeshEvent(const RMeshEvent& event)
	{
		auto [mesh] = mEntityDatabase.getComponents<MeshComponent>(event.getEntity(), true);
		if (mesh) {
			switch (event.getOperation()) {
				case RMeshEvent::Operation::Add:
					for (auto& shadow : mShadows) {
						shadow.uniformUpdater->addRenderable(mesh->get(event.getRIndex()));
					}
					break;
				case RMeshEvent::Operation::Remove:
					for (auto& shadow : mShadows) {
						shadow.uniformUpdater->removeRenderable(mesh->get(event.getRIndex()));
					}
					break;
			}
		}
	}


	void ShadowSystem::onRenderableShaderEvent(const RenderableShaderEvent& event)
	{
		if (event.getRComponentType() == RenderableShaderEvent::RComponentType::Mesh) {
			auto [mesh] = mEntityDatabase.getComponents<MeshComponent>(event.getEntity(), true);
			if (mesh) {
				switch (event.getOperation()) {
					case RenderableShaderEvent::Operation::Add:
						for (auto& shadow : mShadows) {
							shadow.uniformUpdater->addRenderableShader(mesh->get(event.getRIndex()), event.getShader());
						}
						break;
					case RenderableShaderEvent::Operation::Remove:
						for (auto& shadow : mShadows) {
							shadow.uniformUpdater->removeRenderableShader(mesh->get(event.getRIndex()), event.getShader());
						}
						break;
				}
			}
		}
		else {
			graphics::Renderable* renderable = nullptr;
			if (event.getRComponentType() == RenderableShaderEvent::RComponentType::Terrain) {
				auto [terrain] = mEntityDatabase.getComponents<TerrainComponent>(event.getEntity(), true);
				renderable = &terrain->get();
			}

			if (renderable) {
				switch (event.getOperation()) {
					case RenderableShaderEvent::Operation::Add:
						for (auto& shadow : mShadows) {
							shadow.uniformUpdater->addRenderableShader(*renderable, event.getShader());
						}
						break;
					case RenderableShaderEvent::Operation::Remove:
						for (auto& shadow : mShadows) {
							shadow.uniformUpdater->removeRenderableShader(*renderable, event.getShader());
						}
						break;
				}
			}
		}
	}


	void ShadowSystem::onShaderEvent(const ShaderEvent& event)
	{
		switch (event.getOperation()) {
			case ShaderEvent::Operation::Add:
				for (auto& shadow : mShadows) {
					shadow.uniformUpdater->onAddShaderStep(event.getShader(), event.getStep());
				}
				break;
			case ShaderEvent::Operation::Remove:
				for (auto& shadow : mShadows) {
					shadow.uniformUpdater->onRemoveShaderStep(event.getShader(), event.getStep());
				}
				break;
		}
	}

}
