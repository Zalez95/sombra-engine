#include "se/app/events/LightSourceEvent.h"
#include "se/app/events/RenderableShaderEvent.h"
#include "se/app/LightComponent.h"

namespace se::app {

	void LightSource::setType(Type type)
	{
		mType = type;
		mEventManager.publish(new LightSourceEvent(shared_from_this()));
	}


	void LightSource::getShadows(
		std::size_t& shadowResolution,
		float& shadowZNear, float& shadowZFar,
		float& shadowSize, std::size_t& numCascades
	) const
	{
		shadowResolution = mShadowResolution;
		shadowZNear = mShadowZNear;
		shadowZFar = mShadowZFar;
		shadowSize = mShadowSize;
		numCascades = mNumCascades;
	}


	void LightSource::setShadows(
		std::size_t shadowResolution, float shadowZNear, float shadowZFar,
		float shadowSize, std::size_t numCascades
	) {
		mCastShadows = true;
		mShadowResolution = shadowResolution;
		mShadowZNear = shadowZNear;
		mShadowZFar = shadowZFar;
		mShadowSize = shadowSize;
		mNumCascades = numCascades;
		mEventManager.publish(new LightSourceEvent(shared_from_this()));
	}


	void LightSource::disableShadows()
	{
		mCastShadows = false;
		mEventManager.publish(new LightSourceEvent(shared_from_this()));
	}


	LightComponent::LightComponent(const LightComponent& other) :
		mRenderable(other.mRenderable), mSource(other.mSource), mShaders(other.mShaders)
	{
		mRenderable.processTechniques([&](const auto& technique) {
			technique->processPasses([&](const auto& pass) {
				mRenderable.clearBindables(pass.get());
			});
		});
		processRenderableShaders([&](const RenderableShaderRef& shader) {
			shader->processSteps([&](const RenderableShader::StepRef& step) {
				mRenderable.getRenderableMesh().clearBindables(step->getPass().get());
			});
		});
	}


	LightComponent& LightComponent::operator=(const LightComponent& other)
	{
		mEventManager = nullptr;
		mEntity = kNullEntity;
		mRenderable = other.mRenderable;
		mSource = other.mSource;
		mShaders = other.mShaders;

		mRenderable.processTechniques([&](const auto& technique) {
			technique->processPasses([&](const auto& pass) {
				mRenderable.clearBindables(pass.get());
			});
		});
		processRenderableShaders([&](const RenderableShaderRef& shader) {
			shader->processSteps([&](const RenderableShader::StepRef& step) {
				mRenderable.getRenderableMesh().clearBindables(step->getPass().get());
			});
		});

		return *this;
	}


	void LightComponent::setSource(const Repository::ResourceRef<LightSource>& source)
	{
		mSource = source;
		if (mEventManager) {
			mEventManager->publish(new LightSourceEvent(source.get(), mEntity));
		}
	}


	void LightComponent::addRenderableShader(const RenderableShaderRef& shader)
	{
		mShaders.emplace_back(shader);
		mRenderable.getRenderableMesh().addTechnique(shader->getTechnique());
		if (mEventManager) {
			mEventManager->publish(new RenderableShaderEvent(
				RenderableShaderEvent::Operation::Add, mEntity, RenderableShaderEvent::RComponentType::Light, shader.get()
			));
		}
	}


	void LightComponent::removeRenderableShader(const RenderableShaderRef& shader)
	{
		if (mEventManager) {
			mEventManager->publish(new RenderableShaderEvent(
				RenderableShaderEvent::Operation::Remove, mEntity, RenderableShaderEvent::RComponentType::Light, shader.get()
			));
		}
		mRenderable.getRenderableMesh().removeTechnique(shader->getTechnique());
		mShaders.erase(std::remove(mShaders.begin(), mShaders.end(), shader), mShaders.end());
	}

}
