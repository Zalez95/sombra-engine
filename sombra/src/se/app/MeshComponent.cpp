#include "se/app/MeshComponent.h"
#include "se/app/events/RenderableShaderEvent.h"

namespace se::app {

	void MeshComponent::remove(std::size_t rIndex)
	{
		mRMeshes.erase(mRMeshes.begin() + rIndex);
		mEventManager.publish(new RMeshEvent(RMeshEvent::Operation::Remove, mEntity, rIndex));
	}


	void MeshComponent::addRenderableShader(std::size_t rIndex, const RenderableShaderSPtr& shader)
	{
		mRMeshes[rIndex].shaders.emplace_back(shader);
		mRMeshes[rIndex].renderable.addTechnique(shader->getTechnique());
		mEventManager.publish(new RenderableShaderEvent(RenderableShaderEvent::Operation::Add, mEntity, rIndex, shader));
	}


	void MeshComponent::removeRenderableShader(std::size_t rIndex, const RenderableShaderSPtr& shader)
	{
		mRMeshes[rIndex].shaders.erase(
			std::remove(mRMeshes[rIndex].shaders.begin(), mRMeshes[rIndex].shaders.end(), shader),
			mRMeshes[rIndex].shaders.end()
		);
		mRMeshes[rIndex].renderable.removeTechnique(shader->getTechnique());
		mEventManager.publish(new RenderableShaderEvent(RenderableShaderEvent::Operation::Remove, mEntity, rIndex, shader));
	}

}
