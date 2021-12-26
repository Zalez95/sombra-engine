#include "se/utils/Log.h"
#include "se/graphics/BindableRenderNode.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/app/LightProbeSystem.h"
#include "se/app/Application.h"
#include "se/app/TransformsComponent.h"
#include "graphics/DeferredAmbientRenderer.h"

namespace se::app {

	LightProbeSystem::LightProbeSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application), mLightProbeEntity(kNullEntity)
	{
		mApplication.getEntityDatabase().addSystem(this, EntityDatabase::ComponentMask().set<LightProbeComponent>());
	}


	LightProbeSystem::~LightProbeSystem()
	{
		mApplication.getEntityDatabase().removeSystem(this);
	}


	void LightProbeSystem::onNewComponent(
		Entity entity, const EntityDatabase::ComponentMask& mask,
		EntityDatabase::Query& query
	) {
		tryCallC(&LightProbeSystem::onNewLightProbe, entity, mask, query);
	}


	void LightProbeSystem::onRemoveComponent(
		Entity entity, const EntityDatabase::ComponentMask& mask,
		EntityDatabase::Query& query
	) {
		tryCallC(&LightProbeSystem::onRemoveLightProbe, entity, mask, query);
	}


	void LightProbeSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the LightProbes";

		// Update light probe
		mEntityDatabase.executeQuery([this](EntityDatabase::Query& query) {
			if (mLightProbeEntity != kNullEntity) {
				auto [lightProbe] = query.getComponents<LightProbeComponent>(mLightProbeEntity, true);
				if (lightProbe->irradianceMap && lightProbe->prefilterMap
					&& ((mLastIrradianceTexture != lightProbe->irradianceMap.get())
						|| (mLastPrefilterTexture != lightProbe->prefilterMap.get()))
				) {
					auto graphicsEngine = mApplication.getExternalTools().graphicsEngine;
					auto resources = dynamic_cast<graphics::BindableRenderNode*>(graphicsEngine->getRenderGraph().getNode("resources"));

					auto irradianceTexture = dynamic_cast<graphics::BindableRNodeOutput<graphics::Texture>*>(resources->findOutput("irradianceTexture"));
					auto prefilterTexture = dynamic_cast<graphics::BindableRNodeOutput<graphics::Texture>*>(resources->findOutput("prefilterTexture"));
					resources->setBindable(irradianceTexture->getBindableIndex(), lightProbe->irradianceMap.get());
					resources->setBindable(prefilterTexture->getBindableIndex(), lightProbe->prefilterMap.get());
					mLastIrradianceTexture = lightProbe->irradianceMap.get();
					mLastPrefilterTexture = lightProbe->prefilterMap.get();
				}
			}
		});

		SOMBRA_DEBUG_LOG << "Update end";
	}

// Private functions
	void LightProbeSystem::onNewLightProbe(Entity entity, LightProbeComponent* lightProbe, EntityDatabase::Query&)
	{
		mLightProbeEntity = entity;
		SOMBRA_INFO_LOG << "Entity " << entity << " with LightProbe " << lightProbe << " added successfully";
	}


	void LightProbeSystem::onRemoveLightProbe(Entity entity, LightProbeComponent* lightProbe, EntityDatabase::Query&)
	{
		if (mLightProbeEntity == entity) {
			mLightProbeEntity = kNullEntity;

			auto graphicsEngine = mApplication.getExternalTools().graphicsEngine;
			auto resources = dynamic_cast<graphics::BindableRenderNode*>(graphicsEngine->getRenderGraph().getNode("resources"));

			auto irradianceTexture = dynamic_cast<graphics::BindableRNodeOutput<graphics::Texture>*>(resources->findOutput("irradianceTexture"));
			auto prefilterTexture = dynamic_cast<graphics::BindableRNodeOutput<graphics::Texture>*>(resources->findOutput("prefilterTexture"));
			resources->setBindable(irradianceTexture->getBindableIndex(), nullptr);
			resources->setBindable(prefilterTexture->getBindableIndex(), nullptr);
			mLastIrradianceTexture = nullptr;
			mLastPrefilterTexture = nullptr;
		}

		SOMBRA_INFO_LOG << "Entity " << entity << " with LightProbe " << lightProbe << " removed successfully";
	}

}
