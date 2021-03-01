#include <algorithm>
#include "se/app/MeshComponent.h"
#include "se/app/events/RenderableShaderEvent.h"

namespace se::app {

	bool MeshComponent::full() const
	{
		return std::all_of(mRMeshes.begin(), mRMeshes.end(), [](const RMesh& rMesh) { return rMesh.active; });
	}


	std::size_t MeshComponent::add(
		bool hasSkinning,
		std::shared_ptr<graphics::Mesh> mesh, graphics::PrimitiveType primitiveType
	) {
		std::size_t ret = kMaxMeshes;

		auto it = std::find_if(mRMeshes.begin(), mRMeshes.end(), [](const RMesh& rMesh) { return !rMesh.active; });
		if (it != mRMeshes.end()) {
			it->active = true;
			it->hasSkinning = hasSkinning;
			it->renderable = graphics::RenderableMesh(std::move(mesh), primitiveType);
			ret = std::distance(mRMeshes.begin(), it);

			mEventManager.publish(new RMeshEvent(RMeshEvent::Operation::Add, mEntity, ret));
		}

		return ret;
	}


	void MeshComponent::remove(std::size_t rIndex)
	{
		mEventManager.publish(new RMeshEvent(RMeshEvent::Operation::Remove, mEntity, rIndex));
		mRMeshes[rIndex] = {};
	}


	void MeshComponent::addRenderableShader(std::size_t rIndex, const RenderableShaderSPtr& shader)
	{
		mRMeshes[rIndex].shaders.emplace_back(shader);
		mRMeshes[rIndex].renderable.addTechnique(shader->getTechnique());
		mEventManager.publish(new RenderableShaderEvent(RenderableShaderEvent::Operation::Add, mEntity, rIndex, shader));
	}


	void MeshComponent::removeRenderableShader(std::size_t rIndex, const RenderableShaderSPtr& shader)
	{
		mEventManager.publish(new RenderableShaderEvent(RenderableShaderEvent::Operation::Remove, mEntity, rIndex, shader));
		mRMeshes[rIndex].renderable.removeTechnique(shader->getTechnique());
		mRMeshes[rIndex].shaders.erase(
			std::remove(mRMeshes[rIndex].shaders.begin(), mRMeshes[rIndex].shaders.end(), shader),
			mRMeshes[rIndex].shaders.end()
		);
	}

}
