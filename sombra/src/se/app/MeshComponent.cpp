#include <algorithm>
#include "se/app/events/RMeshEvent.h"
#include "se/app/MeshComponent.h"
#include "se/app/events/RenderableShaderEvent.h"

namespace se::app {

	MeshComponent::MeshComponent(const MeshComponent& other) : mRMeshes(other.mRMeshes)
	{
		processRenderableIndices([&](std::size_t i) {
			processRenderableShaders(i, [&](const RenderableShaderRef& shader) {
				shader->processSteps([&](const RenderableShader::StepRef& step) {
					mRMeshes[i].renderable.clearBindables(step->getPass().get());
				});
			});
		});
	}


	MeshComponent& MeshComponent::operator=(const MeshComponent& other)
	{
		mEventManager = nullptr;
		mEntity = kNullEntity;
		mRMeshes = other.mRMeshes;

		processRenderableIndices([&](std::size_t i) {
			processRenderableShaders(i, [&](const RenderableShaderRef& shader) {
				shader->processSteps([&](const RenderableShader::StepRef& step) {
					mRMeshes[i].renderable.clearBindables(step->getPass().get());
				});
			});
		});

		return *this;
	}


	bool MeshComponent::full() const
	{
		return std::all_of(mRMeshes.begin(), mRMeshes.end(), [](const RMesh& rMesh) { return rMesh.active; });
	}


	void MeshComponent::setMesh(std::size_t rIndex, MeshRef mesh)
	{
		mRMeshes[rIndex].mesh = mesh;
		mRMeshes[rIndex].renderable.setMesh(mesh.get());
	}


	std::size_t MeshComponent::add(bool hasSkinning, MeshRef mesh, graphics::PrimitiveType primitiveType)
	{
		std::size_t ret = kMaxMeshes;

		auto it = std::find_if(mRMeshes.begin(), mRMeshes.end(), [](const RMesh& rMesh) { return !rMesh.active; });
		if (it != mRMeshes.end()) {
			it->active = true;
			it->hasSkinning = hasSkinning;
			it->mesh = mesh;
			it->renderable = graphics::RenderableMesh(mesh.get(), primitiveType);
			ret = std::distance(mRMeshes.begin(), it);

			if (mEventManager) {
				mEventManager->publish(new RMeshEvent(RMeshEvent::Operation::Add, mEntity, ret));
			}
		}

		return ret;
	}


	void MeshComponent::remove(std::size_t rIndex)
	{
		if (mEventManager) {
			mEventManager->publish(new RMeshEvent(RMeshEvent::Operation::Remove, mEntity, rIndex));
		}
		mRMeshes[rIndex] = {};
	}


	void MeshComponent::clear()
	{
		if (mEventManager) {
			for (std::size_t i = 0; i < kMaxMeshes; ++i) {
				if (mRMeshes[i].active) {
					mEventManager->publish(new RMeshEvent(RMeshEvent::Operation::Remove, mEntity, i));
					mRMeshes[i] = {};
				}
			}
		}
	}


	void MeshComponent::addRenderableShader(std::size_t rIndex, const RenderableShaderRef& shader)
	{
		mRMeshes[rIndex].shaders.emplace_back(shader);
		mRMeshes[rIndex].renderable.addTechnique(shader->getTechnique());
		if (mEventManager) {
			mEventManager->publish(new RenderableShaderEvent(RenderableShaderEvent::Operation::Add, mEntity, rIndex, shader.get()));
		}
	}


	void MeshComponent::removeRenderableShader(std::size_t rIndex, const RenderableShaderRef& shader)
	{
		if (mEventManager) {
			mEventManager->publish(new RenderableShaderEvent(RenderableShaderEvent::Operation::Remove, mEntity, rIndex, shader.get()));
		}
		mRMeshes[rIndex].renderable.removeTechnique(shader->getTechnique());
		mRMeshes[rIndex].shaders.erase(
			std::remove(mRMeshes[rIndex].shaders.begin(), mRMeshes[rIndex].shaders.end(), shader),
			mRMeshes[rIndex].shaders.end()
		);
	}

}
