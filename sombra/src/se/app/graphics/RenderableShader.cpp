#include "se/app/events/ShaderEvent.h"
#include "se/app/graphics/RenderableShader.h"

namespace se::app {

	template <>
	RenderableShaderStep& RenderableShaderStep::addResource<graphics::Program>(const ProgramRef& resource, bool addResource)
	{
		mPrograms.push_back(resource);
		if (addResource) {
			addBindable(resource.get());
		}
		return *this;
	}


	template <>
	RenderableShaderStep& RenderableShaderStep::addResource<graphics::Texture>(const TextureRef& resource, bool addResource)
	{
		mTextures.push_back(resource);
		if (addResource) {
			addBindable(resource.get());
		}
		return *this;
	}


	template <>
	RenderableShaderStep& RenderableShaderStep::removeResource<graphics::Program>(const ProgramRef& resource, bool removeResource)
	{
		mPrograms.erase(std::remove(mPrograms.begin(), mPrograms.end(), resource), mPrograms.end());
		if (removeResource) {
			removeBindable(resource.get());
		}
		return *this;
	}


	template <>
	RenderableShaderStep& RenderableShaderStep::removeResource<graphics::Texture>(const TextureRef& resource, bool removeResource)
	{
		mTextures.erase(std::remove(mTextures.begin(), mTextures.end(), resource), mTextures.end());
		if (removeResource) {
			removeBindable(resource.get());
		}
		return *this;
	}


	RenderableShaderStep& RenderableShaderStep::addBindable(const BindableSPtr& bindable)
	{
		mPass->addBindable(bindable);
		return *this;
	}


	RenderableShaderStep& RenderableShaderStep::removeBindable(const BindableSPtr& bindable)
	{
		mPass->removeBindable(bindable);
		return *this;
	}


	RenderableShader& RenderableShader::addStep(const StepRef& step)
	{
		mSteps.push_back(step);
		mTechnique->addPass(step->getPass());
		mEventManager.publish(new ShaderEvent(ShaderEvent::Operation::Add, shared_from_this(), step.get()));
		return *this;
	}


	RenderableShader& RenderableShader::removeStep(const StepRef& step)
	{
		mEventManager.publish(new ShaderEvent(ShaderEvent::Operation::Remove, shared_from_this(), step.get()));
		mTechnique->removePass(step->getPass());
		mSteps.erase(std::remove(mSteps.begin(), mSteps.end(), step), mSteps.end());
		return *this;
	}

}
