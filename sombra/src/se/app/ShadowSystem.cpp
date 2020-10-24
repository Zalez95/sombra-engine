#include "se/utils/Log.h"
#include "se/app/ShadowSystem.h"
#include "se/app/Application.h"
#include "se/app/EntityDatabase.h"
#include "se/app/TransformsComponent.h"
#include "se/graphics/Renderer.h"
#include "se/graphics/RenderGraph.h"
#include "se/graphics/GraphicsEngine.h"

namespace se::app {

	ShadowSystem::ShadowSystem(Application& application, const ShadowData& shadowData) :
		IVPSystem(application, "uViewMatrix", "uProjectionMatrix"),
		mShadowData(shadowData), mShadowEntity(kNullEntity)
	{
		mApplication.getEventManager().subscribe(this, Topic::Shadow);
		mEntityDatabase.addSystem(this, mEntityDatabase.getSystemMask(this).set<LightComponent>());

		auto& renderGraph = mApplication.getExternalTools().graphicsEngine->getRenderGraph();
		mShadowRenderer = renderGraph.getNode("shadowRenderer");
	}


	ShadowSystem::~ShadowSystem()
	{
		mEntityDatabase.removeSystem(this);
		mApplication.getEventManager().unsubscribe(this, Topic::Shadow);
	}


	void ShadowSystem::notify(const IEvent& event)
	{
		tryCall(&ShadowSystem::onShadowEvent, event);
	}


	void ShadowSystem::onRemoveEntity(Entity entity)
	{
		if (mShadowEntity == entity) {
			mShadowEntity = kNullEntity;
			SOMBRA_INFO_LOG << "Active Shadow Entity " << entity << " removed";
		}

		IVPSystem::onRemoveEntity(entity);

		SOMBRA_INFO_LOG << "Entity " << entity << " removed successfully";
	}


	void ShadowSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Cameras";

		auto [transforms] = mEntityDatabase.getComponents<TransformsComponent>(mShadowEntity);
		if (transforms && transforms->updated.any()) {
			mShadowCamera.setPosition(transforms->position);
			mShadowCamera.setTarget(transforms->position + glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation);
			mShadowCamera.setUp({ 0.0f, 1.0f, 0.0f });
		}

		IVPSystem::update();

		SOMBRA_INFO_LOG << "Update end";
	}

// Private functions
	glm::mat4 ShadowSystem::getViewMatrix() const
	{
		if (mShadowEntity != kNullEntity) {
			return mShadowCamera.getViewMatrix();
		}
		return glm::mat4(1.0f);
	}


	glm::mat4 ShadowSystem::getProjectionMatrix() const
	{
		if (mShadowEntity != kNullEntity) {
			return mShadowCamera.getProjectionMatrix();
		}
		return glm::mat4(1.0f);
	}


	bool ShadowSystem::shouldAddUniforms(PassSPtr pass) const
	{
		return (&pass->getRenderer() == mShadowRenderer);
	}


	void ShadowSystem::onShadowEvent(const ContainerEvent<Topic::Shadow, Entity>& event)
	{
		auto [transforms, light] = mEntityDatabase.getComponents<TransformsComponent, LightComponent>(event.getValue());
		if (transforms && light) {
			mShadowEntity = event.getValue();

			mShadowCamera.setPosition(transforms->position);
			mShadowCamera.setTarget(transforms->position + glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation);
			mShadowCamera.setUp({ 0.0f, 1.0f, 0.0f });

			if (light->source->type == LightSource::Type::Directional) {
				mShadowCamera.setOrthographicProjectionMatrix(
					-mShadowData.size, mShadowData.size, -mShadowData.size, mShadowData.size,
					mShadowData.zNear, mShadowData.zFar
				);
			}
			else if (light->source->type == LightSource::Type::Spot) {
				mShadowCamera.setPerspectiveProjectionMatrix(
					glm::radians(45.0f), 1.0f,
					mShadowData.zNear, mShadowData.zFar
				);
			}
		}
		else {
			SOMBRA_WARN_LOG << "Couldn't set Entity " << event.getValue() << " as Camera";
		}
	}

}
