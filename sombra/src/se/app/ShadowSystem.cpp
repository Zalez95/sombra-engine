#include "se/utils/Log.h"
#include "se/graphics/Renderer.h"
#include "se/graphics/RenderGraph.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/graphics/3D/RenderableTerrain.h"
#include "se/app/ShadowSystem.h"
#include "se/app/Application.h"
#include "se/app/EntityDatabase.h"
#include "se/app/TransformsComponent.h"
#include "se/app/MeshComponent.h"
#include "se/app/CameraComponent.h"
#include "se/app/graphics/ShadowRenderer3D.h"
#include "se/app/graphics/IViewProjectionUpdater.h"

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

		virtual bool shouldAddUniforms(PassSPtr pass) const override
		{
			return (&pass->getRenderer() == mParent->mShadows[mShadowIndex].renderer);
		};
	};


	ShadowSystem::ShadowSystem(Application& application, const ShadowData& shadowData) :
		ISystem(application.getEntityDatabase()), mApplication(application)
	{
		mApplication.getEventManager().subscribe(this, Topic::Shadow);
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask()
			.set<LightComponent>()
			.set<MeshComponent>()
			.set<graphics::RenderableTerrain>()
		);

		auto& renderGraph = mApplication.getExternalTools().graphicsEngine->getRenderGraph();

		auto& shadow = mShadows.emplace_back();
		shadow.data = shadowData;
		shadow.uniformUpdater = new ShadowUniformsUpdater(this, 0, "uViewMatrix", "uProjectionMatrix");
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
		mApplication.getEventManager().unsubscribe(this, Topic::Shadow);
	}


	void ShadowSystem::notify(const IEvent& event)
	{
		tryCall(&ShadowSystem::onShadowEvent, event);
	}


	void ShadowSystem::onNewEntity(Entity entity)
	{
		auto [mesh, rTerrain] = mEntityDatabase.getComponents<
			MeshComponent, graphics::RenderableTerrain
		>(entity);

		if (mesh) {
			for (auto& rMesh : mesh->rMeshes) {
				for (auto& shadow : mShadows) {
					shadow.uniformUpdater->addRenderable(rMesh);
				}
			}
		}
		if (rTerrain) {
			for (auto& shadow : mShadows) {
				shadow.uniformUpdater->addRenderable(*rTerrain);
			}
		}
	}


	void ShadowSystem::onRemoveEntity(Entity entity)
	{
		auto [mesh, rTerrain] = mEntityDatabase.getComponents<
			MeshComponent, graphics::RenderableTerrain
		>(entity);

		if (mesh) {
			for (auto& rMesh : mesh->rMeshes) {
				for (auto& shadow : mShadows) {
					shadow.uniformUpdater->removeRenderable(rMesh);
				}
			}
		}
		if (rTerrain) {
			for (auto& shadow : mShadows) {
				shadow.uniformUpdater->removeRenderable(*rTerrain);
			}
		}

		SOMBRA_INFO_LOG << "Entity " << entity << " removed successfully";
	}


	void ShadowSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Cameras";

		auto [transforms] = mEntityDatabase.getComponents<TransformsComponent>(mShadowEntity);
		if (transforms && transforms->updated.any()) {
			mShadows[0].camera.setPosition(transforms->position);
			mShadows[0].camera.setTarget(transforms->position + glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation);
			mShadows[0].camera.setUp({ 0.0f, 1.0f, 0.0f });
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

			mShadows[0].camera.setPosition(transforms->position);
			mShadows[0].camera.setTarget(transforms->position + glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation);
			mShadows[0].camera.setUp({ 0.0f, 1.0f, 0.0f });

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

}
