#include "se/app/TerrainComponent.h"
#include "se/app/events/RenderableShaderEvent.h"

namespace se::app {

	void TerrainComponent::addRenderableShader(const RenderableShaderSPtr& shader)
	{
		mShaders.emplace_back(shader);
		mRenderableTerrain.addTechnique(shader->getTechnique());
		mEventManager.publish(new RenderableShaderEvent(RenderableShaderEvent::Operation::Add, mEntity, shader));
	}


	void TerrainComponent::removeRenderableShader(const RenderableShaderSPtr& shader)
	{
		mShaders.erase(std::remove(mShaders.begin(), mShaders.end(), shader), mShaders.end());
		mRenderableTerrain.removeTechnique(shader->getTechnique());
		mEventManager.publish(new RenderableShaderEvent(RenderableShaderEvent::Operation::Remove, mEntity, shader));
	}

}
