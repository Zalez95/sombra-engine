#include "se/app/TerrainComponent.h"
#include "se/app/events/RenderableShaderEvent.h"

namespace se::app {

	TerrainComponent::TerrainComponent(const TerrainComponent& other) :
		mRenderableTerrain(other.mRenderableTerrain), mShaders(other.mShaders)
	{
		processRenderableShaders([&](const RenderableShaderResource& shader) {
			shader->processSteps([&](const RenderableShader::StepResource& step) {
				mRenderableTerrain.clearBindables(step->getPass().get());
			});
		});
	}


	TerrainComponent& TerrainComponent::operator=(const TerrainComponent& other)
	{
		mEventManager = nullptr;
		mEntity = kNullEntity;
		mRenderableTerrain = other.mRenderableTerrain;
		mShaders = other.mShaders;

		processRenderableShaders([&](const RenderableShaderResource& shader) {
			shader->processSteps([&](const RenderableShader::StepResource& step) {
				mRenderableTerrain.clearBindables(step->getPass().get());
			});
		});

		return *this;
	}


	void TerrainComponent::addRenderableShader(const RenderableShaderResource& shader)
	{
		mShaders.emplace_back(shader);
		mRenderableTerrain.addTechnique(shader->getTechnique());
		if (mEventManager) {
			mEventManager->publish(std::make_unique<RenderableShaderEvent>(
				RenderableShaderEvent::Operation::Add, mEntity, RenderableShaderEvent::RComponentType::Terrain, shader.get()
			));
		}
	}


	void TerrainComponent::removeRenderableShader(const RenderableShaderResource& shader)
	{
		if (mEventManager) {
			mEventManager->publish(std::make_unique<RenderableShaderEvent>(
				RenderableShaderEvent::Operation::Remove, mEntity, RenderableShaderEvent::RComponentType::Terrain, shader.get()
			));
		}
		mRenderableTerrain.removeTechnique(shader->getTechnique());
		mShaders.erase(std::remove(mShaders.begin(), mShaders.end(), shader), mShaders.end());
	}

}
