#include <array>
#include <memory>
#include "se/utils/Log.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/app/graphics/AppRenderGraph.h"
#include "se/app/AppRenderer.h"
#include "se/app/Application.h"
#include "se/app/TransformsComponent.h"
#include "se/app/events/ContainerEvent.h"

namespace se::app {

	AppRenderer::AppRenderer(Application& application, const ShadowData& shadowData, std::size_t width, std::size_t height) :
		ISystem(application.getEntityDatabase()), mApplication(application),
		mDeferredLightRenderer(nullptr), mLastIrradianceTexture(nullptr), mLastPrefilterTexture(nullptr),
		mShadowEntity(kNullEntity), mLightProbeEntity(kNullEntity)
	{
		mApplication.getEventManager().subscribe(this, Topic::WindowResize);
		mApplication.getEventManager().subscribe(this, Topic::RendererResolution);
		mApplication.getEventManager().subscribe(this, Topic::Shadow);
		mApplication.getEntityDatabase().addSystem(this, EntityDatabase::ComponentMask().set<LightComponent>().set<LightProbe>());

		SOMBRA_INFO_LOG << graphics::GraphicsOperations::getGraphicsInfo();
		auto graph = std::make_unique<AppRenderGraph>(application.getRepository(), shadowData, width, height);
		mDeferredLightRenderer = dynamic_cast<DeferredLightRenderer*>(graph->getNode("deferredLightRenderer"));
		mApplication.getExternalTools().graphicsEngine->setRenderGraph(std::move(graph));

		graphics::GraphicsOperations::setViewport(0, 0, width, height);
	}


	AppRenderer::~AppRenderer()
	{
		mApplication.getExternalTools().graphicsEngine->getRenderGraph().clearNodes();
		mApplication.getRepository().findByName<graphics::Mesh>("plane").setFakeUser(false);
		mApplication.getEntityDatabase().removeSystem(this);
		mApplication.getEventManager().unsubscribe(this, Topic::Shadow);
		mApplication.getEventManager().unsubscribe(this, Topic::RendererResolution);
		mApplication.getEventManager().unsubscribe(this, Topic::WindowResize);
	}


	bool AppRenderer::notify(const IEvent& event)
	{
		return tryCall(&AppRenderer::onShadowEvent, event)
			|| tryCall(&AppRenderer::onWindowResizeEvent, event)
			|| tryCall(&AppRenderer::onRendererResolutionEvent, event);
	}


	void AppRenderer::onNewComponent(Entity entity, const EntityDatabase::ComponentMask& mask)
	{
		tryCallC(&AppRenderer::onNewLight, entity, mask);
		tryCallC(&AppRenderer::onNewLightProbe, entity, mask);
	}


	void AppRenderer::onRemoveComponent(Entity entity, const EntityDatabase::ComponentMask& mask)
	{
		tryCallC(&AppRenderer::onRemoveLight, entity, mask);
		tryCallC(&AppRenderer::onRemoveLightProbe, entity, mask);
	}


	void AppRenderer::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the LightComponents";

		// Update light probe
		if (mLightProbeEntity != kNullEntity) {
			auto [lightProbe] = mEntityDatabase.getComponents<LightProbe>(mLightProbeEntity, true);
			if ((mLastIrradianceTexture != lightProbe->irradianceMap.get().get())
				|| (mLastPrefilterTexture != lightProbe->prefilterMap.get().get())
			) {
				auto graphicsEngine = mApplication.getExternalTools().graphicsEngine;
				auto resources = dynamic_cast<graphics::BindableRenderNode*>(graphicsEngine->getRenderGraph().getNode("resources"));

				auto irradianceTexture = dynamic_cast<graphics::BindableRNodeOutput<graphics::Texture>*>(resources->findOutput("irradianceTexture"));
				auto prefilterTexture = dynamic_cast<graphics::BindableRNodeOutput<graphics::Texture>*>(resources->findOutput("prefilterTexture"));
				resources->setBindable(irradianceTexture->getBindableIndex(), lightProbe->irradianceMap.get());
				resources->setBindable(prefilterTexture->getBindableIndex(), lightProbe->prefilterMap.get());
				mLastIrradianceTexture = lightProbe->irradianceMap.get().get();
				mLastPrefilterTexture = lightProbe->prefilterMap.get().get();
			}
		}

		// Update light sources and shadows
		unsigned int i = 0, iShadowLight = DeferredLightRenderer::kMaxLights;
		std::array<DeferredLightRenderer::ShaderLightSource, DeferredLightRenderer::kMaxLights> uBaseLights;
		mEntityDatabase.iterateComponents<TransformsComponent, LightComponent>(
			[&](Entity entity, TransformsComponent* transforms, LightComponent* light) {
				if (light->source && (i < DeferredLightRenderer::kMaxLights)) {
					uBaseLights[i].type = static_cast<unsigned int>(light->source->type);
					uBaseLights[i].position = transforms->position;
					uBaseLights[i].direction = glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation);
					uBaseLights[i].color = { light->source->color, 1.0f };
					uBaseLights[i].intensity = light->source->intensity;
					switch (light->source->type) {
						case LightSource::Type::Directional: {
							uBaseLights[i].range = std::numeric_limits<float>::max();
						} break;
						case LightSource::Type::Point: {
							uBaseLights[i].range = light->source->range;
						} break;
						case LightSource::Type::Spot: {
							float cosInner = std::cos(light->source->innerConeAngle);
							float cosOuter = std::cos(light->source->outerConeAngle);
							uBaseLights[i].range = light->source->range;
							uBaseLights[i].lightAngleScale = 1.0f / std::max(0.001f, cosInner - cosOuter);
							uBaseLights[i].lightAngleOffset = -cosOuter * uBaseLights[i].lightAngleScale;
						} break;
					}

					if (mShadowEntity == entity) {
						iShadowLight = i;
					}
					++i;
				}
			},
			true
		);

		mDeferredLightRenderer->setLights(uBaseLights.data(), i);
		mDeferredLightRenderer->setShadowLightIndex(iShadowLight);

		SOMBRA_INFO_LOG << "Update end";
	}


	void AppRenderer::render()
	{
		SOMBRA_INFO_LOG << "Render start";
		mApplication.getExternalTools().graphicsEngine->render();
		SOMBRA_INFO_LOG << "Render end";
	}

// Private functions
	void AppRenderer::onNewLight(Entity entity, LightComponent* light)
	{
		SOMBRA_INFO_LOG << "Entity " << entity << " with LightComponent " << light << " added successfully";
	}


	void AppRenderer::onRemoveLight(Entity entity, LightComponent* light)
	{
		if (mShadowEntity == entity) {
			mShadowEntity = kNullEntity;
			SOMBRA_INFO_LOG << "Active Shadow Camera removed";
		}

		SOMBRA_INFO_LOG << "Entity " << entity << " with LightComponent " << light << " removed successfully";
	}


	void AppRenderer::onNewLightProbe(Entity entity, LightProbe* lightProbe)
	{
		mLightProbeEntity = entity;
		SOMBRA_INFO_LOG << "Entity " << entity << " with LightProbe " << lightProbe << " added successfully";
	}


	void AppRenderer::onRemoveLightProbe(Entity entity, LightProbe* lightProbe)
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


	void AppRenderer::onShadowEvent(const ContainerEvent<Topic::Shadow, Entity>& event)
	{
		if (mEntityDatabase.hasComponents<TransformsComponent, LightComponent>(event.getValue())) {
			mShadowEntity = event.getValue();
		}
		else {
			SOMBRA_WARN_LOG << "Couldn't set Entity " << event.getValue() << " as Shadow Entity";
		}
	}


	void AppRenderer::onWindowResizeEvent(const WindowResizeEvent& event)
	{
		auto width = static_cast<std::size_t>(event.getWidth());
		auto height = static_cast<std::size_t>(event.getHeight());
		graphics::GraphicsOperations::setViewport(0, 0, width, height);
	}


	void AppRenderer::onRendererResolutionEvent(const RendererResolutionEvent& event)
	{
		auto graphicsEngine = mApplication.getExternalTools().graphicsEngine;
		if (auto graph = dynamic_cast<AppRenderGraph*>(&graphicsEngine->getRenderGraph())) {
			auto width = static_cast<std::size_t>(event.getWidth());
			auto height = static_cast<std::size_t>(event.getHeight());
			graph->setResolution(width, height);
		}
	}

}
