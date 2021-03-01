#include "se/utils/Log.h"
#include "se/graphics/Renderer.h"
#include "se/graphics/RenderGraph.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/app/ShadowSystem.h"
#include "se/app/Application.h"
#include "se/app/EntityDatabase.h"
#include "se/app/TransformsComponent.h"
#include "se/app/MeshComponent.h"
#include "se/app/TerrainComponent.h"
#include "se/app/CameraComponent.h"
#include "se/app/IViewProjectionUpdater.h"
#include "se/app/graphics/ShadowRenderer3D.h"

namespace se::app {

	struct ShadowSystem::Shadow
	{
		ShadowData data;
		ShadowUniformsUpdater* uniformUpdater;
		CameraComponent camera;
		ShadowRenderer3D* renderer;
	};


	class ShadowSystem::ShadowUniformsUpdater : public IViewProjectionUpdater
	{
	private:
		ShadowSystem* mParent;
		std::size_t mShadowIndex;

	public:		// Functions
		ShadowUniformsUpdater(
			ShadowSystem* parent, std::size_t shadowIndex,
			const std::string& viewMatUniformName, const std::string& projectionMatUniformName
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

		virtual bool shouldAddUniforms(const PassSPtr& pass) const override
		{
			return (&pass->getRenderer() == mParent->mShadows[mShadowIndex].renderer);
		};
	};


	ShadowSystem::ShadowSystem(Application& application, const ShadowData& shadowData) :
		ISystem(application.getEntityDatabase()), mApplication(application)
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

		auto& shadow = mShadows.emplace_back();
		shadow.data = shadowData;
		shadow.uniformUpdater = new ShadowUniformsUpdater(this, 0, "uViewMatrix", "uProjectionMatrix");

		auto& renderGraph = mApplication.getExternalTools().graphicsEngine->getRenderGraph();
		shadow.renderer = dynamic_cast<ShadowRenderer3D*>(renderGraph.getNode("shadowRenderer"));
		shadow.renderer->setShadowResolution(shadow.data.resolution);

		mDeferredLightRenderer = dynamic_cast<DeferredLightRenderer*>(renderGraph.getNode("deferredLightRenderer"));
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


	void ShadowSystem::onNewEntity(Entity entity)
	{
		auto [mesh, terrain] = mEntityDatabase.getComponents<MeshComponent, TerrainComponent>(entity);
		if (mesh) {
			mesh->processRenderableIndices([&, mesh = mesh](std::size_t i) {
				for (auto& shadow : mShadows) {
					shadow.uniformUpdater->addRenderable(mesh->get(i));
					mesh->processRenderableShaders(i, [&](const auto& shader) {
						shadow.uniformUpdater->addRenderableShader(mesh->get(i), shader);
					});
				}
			});
		}
		if (terrain) {
			for (auto& shadow : mShadows) {
				shadow.uniformUpdater->addRenderable(terrain->get());
				terrain->processRenderableShaders([&](const auto& shader) {
					shadow.uniformUpdater->addRenderableShader(terrain->get(), shader);
				});
			}
		}
	}


	void ShadowSystem::onRemoveEntity(Entity entity)
	{
		auto [mesh, terrain] = mEntityDatabase.getComponents<MeshComponent, TerrainComponent>(entity);
		if (mesh) {
			mesh->processRenderableIndices([&, mesh = mesh](std::size_t i) {
				for (auto& shadow : mShadows) {
					shadow.uniformUpdater->removeRenderable(mesh->get(i));
				}
			});
		}
		if (terrain) {
			for (auto& shadow : mShadows) {
				shadow.uniformUpdater->removeRenderable(terrain->get());
			}
		}

		SOMBRA_INFO_LOG << "Entity " << entity << " removed successfully";
	}


	void ShadowSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Cameras";

		auto [transforms] = mEntityDatabase.getComponents<TransformsComponent>(mShadowEntity);
		if (transforms && !transforms->updated[static_cast<int>(TransformsComponent::Update::Shadow)]) {
			mShadows[0].camera.setPosition(transforms->position);
			mShadows[0].camera.setOrientation(transforms->orientation);
			transforms->updated.set(static_cast<int>(TransformsComponent::Update::Shadow));
		}

		SOMBRA_DEBUG_LOG << "Updating the Uniforms";
		mShadows[0].uniformUpdater->update();

		SOMBRA_DEBUG_LOG << "Updating the Renderers";
		glm::mat4 viewProjectionMatrix = mShadows[0].camera.getProjectionMatrix() * mShadows[0].camera.getViewMatrix();
		mShadows[0].renderer->updateFrustum(viewProjectionMatrix);
		mDeferredLightRenderer->setShadowViewProjectionMatrix(viewProjectionMatrix);

		SOMBRA_INFO_LOG << "Update end";
	}

// Private functions
	void ShadowSystem::onShadowEvent(const ContainerEvent<Topic::Shadow, Entity>& event)
	{
		auto [transforms, light] = mEntityDatabase.getComponents<TransformsComponent, LightComponent>(event.getValue());
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
		auto [mesh] = mEntityDatabase.getComponents<MeshComponent>(event.getEntity());
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
		if (event.isTerrain()) {
			auto [terrain] = mEntityDatabase.getComponents<TerrainComponent>(event.getEntity());
			if (terrain) {
				switch (event.getOperation()) {
					case RenderableShaderEvent::Operation::Add:
						for (auto& shadow : mShadows) {
							shadow.uniformUpdater->addRenderableShader(terrain->get(), event.getShader());
						}
						break;
					case RenderableShaderEvent::Operation::Remove:
						for (auto& shadow : mShadows) {
							shadow.uniformUpdater->removeRenderableShader(terrain->get(), event.getShader());
						}
						break;
				}
			}
		}
		else {
			auto [mesh] = mEntityDatabase.getComponents<MeshComponent>(event.getEntity());
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
	}


	void ShadowSystem::onShaderEvent(const ShaderEvent& event)
	{
		switch (event.getOperation()) {
			case ShaderEvent::Operation::Add:
				for (auto& shadow : mShadows) {
					shadow.uniformUpdater->onAddShaderPass(event.getShader(), event.getPass());
				}
				break;
			case ShaderEvent::Operation::Remove:
				for (auto& shadow : mShadows) {
					shadow.uniformUpdater->onRemoveShaderPass(event.getShader(), event.getPass());
				}
				break;
		}
	}

}
