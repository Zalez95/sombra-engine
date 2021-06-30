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
#include "se/app/graphics/DeferredLightRenderer.h"

namespace se::app {

	AppRenderer::AppRenderer(Application& application, std::size_t width, std::size_t height) :
		ISystem(application.getEntityDatabase()), mApplication(application),
		mDeferredLightRenderer(nullptr), mLastIrradianceTexture(nullptr), mLastPrefilterTexture(nullptr),
		mLightProbeEntity(kNullEntity)
	{
		mApplication.getEventManager().subscribe(this, Topic::WindowResize);
		mApplication.getEventManager().subscribe(this, Topic::RendererResolution);
		mApplication.getEntityDatabase().addSystem(this, EntityDatabase::ComponentMask().set<LightProbe>());

		SOMBRA_INFO_LOG << graphics::GraphicsOperations::getGraphicsInfo();
		auto graph = std::make_unique<AppRenderGraph>(application.getRepository(), width, height);
		mDeferredLightRenderer = dynamic_cast<DeferredLightRenderer*>(graph->getNode("deferredLightRenderer"));
		mApplication.getExternalTools().graphicsEngine->setRenderGraph(std::move(graph));

		graphics::GraphicsOperations::setViewport(0, 0, width, height);
	}


	AppRenderer::~AppRenderer()
	{
		mApplication.getExternalTools().graphicsEngine->getRenderGraph().clearNodes();
		mApplication.getRepository().findByName<graphics::Mesh>("plane").setFakeUser(false);
		mApplication.getEntityDatabase().removeSystem(this);
		mApplication.getEventManager().unsubscribe(this, Topic::RendererResolution);
		mApplication.getEventManager().unsubscribe(this, Topic::WindowResize);
	}


	bool AppRenderer::notify(const IEvent& event)
	{
		return tryCall(&AppRenderer::onWindowResizeEvent, event)
			|| tryCall(&AppRenderer::onRendererResolutionEvent, event);
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
		unsigned int i = 0;
		std::array<DeferredLightRenderer::ShaderLightSource, DeferredLightRenderer::kMaxLights> uBaseLights;
		mEntityDatabase.iterateComponents<TransformsComponent, LightComponent>(
			[&](Entity, TransformsComponent* transforms, LightComponent* light) {
				if (light->getSource() && (i < DeferredLightRenderer::kMaxLights)) {
					uBaseLights[i].type = static_cast<unsigned int>(light->getSource()->type);
					uBaseLights[i].position = transforms->position;
					uBaseLights[i].direction = glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation);
					uBaseLights[i].shadowIndices = light->getShadowIndices();
					uBaseLights[i].color = { light->getSource()->color, 1.0f };
					uBaseLights[i].intensity = light->getSource()->intensity;
					switch (light->getSource()->type) {
						case LightSource::Type::Directional: {
							uBaseLights[i].range = std::numeric_limits<float>::max();
						} break;
						case LightSource::Type::Point: {
							uBaseLights[i].range = light->getSource()->range;
						} break;
						case LightSource::Type::Spot: {
							float cosInner = std::cos(light->getSource()->innerConeAngle);
							float cosOuter = std::cos(light->getSource()->outerConeAngle);
							uBaseLights[i].range = light->getSource()->range;
							uBaseLights[i].lightAngleScale = 1.0f / std::max(0.001f, cosInner - cosOuter);
							uBaseLights[i].lightAngleOffset = -cosOuter * uBaseLights[i].lightAngleScale;
						} break;
					}
					++i;
				}
			},
			true
		);

		mDeferredLightRenderer->setLights(uBaseLights.data(), i);

		SOMBRA_INFO_LOG << "Update end";
	}


	void AppRenderer::render()
	{
		SOMBRA_INFO_LOG << "Render start";
		mApplication.getExternalTools().graphicsEngine->render();
		SOMBRA_INFO_LOG << "Render end";
	}

// Private functions
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
