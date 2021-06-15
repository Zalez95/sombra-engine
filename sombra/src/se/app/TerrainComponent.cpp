#include "se/app/TerrainComponent.h"
#include "se/app/events/RenderableShaderEvent.h"

namespace se::app {

	void TerrainComponent::addRenderableShader(const RenderableShaderRef& shader)
	{
		mShaders.emplace_back(shader);
		mRenderableTerrain.addTechnique(shader->getTechnique());
		mEventManager.publish(new RenderableShaderEvent(
			RenderableShaderEvent::Operation::Add, mEntity, RenderableShaderEvent::RComponentType::Terrain, shader.get()
		));
	}


	void TerrainComponent::removeRenderableShader(const RenderableShaderRef& shader)
	{
		mEventManager.publish(new RenderableShaderEvent(
			RenderableShaderEvent::Operation::Remove, mEntity, RenderableShaderEvent::RComponentType::Terrain, shader.get()
		));
		mRenderableTerrain.removeTechnique(shader->getTechnique());
		mShaders.erase(std::remove(mShaders.begin(), mShaders.end(), shader), mShaders.end());
	}

}
