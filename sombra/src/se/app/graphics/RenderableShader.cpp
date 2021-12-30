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


	std::unique_ptr<RenderableShaderStep> RenderableShaderStep::clone() const
	{
		auto ret = std::make_unique<RenderableShaderStep>(mPass->getRenderer());
		for (const ProgramRef& program : mPrograms) {
			ret->addResource(program);
		}
		for (const TextureRef& texture : mTextures) {
			ret->addResource(texture);
		}
		processBindables([&](const BindableSPtr& bindable) {
			if (!std::dynamic_pointer_cast<graphics::Texture>(bindable)
				&& !std::dynamic_pointer_cast<graphics::Program>(bindable)
			) {
				ret->addBindable(bindable->clone());
			}
		});
		return ret;
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


	std::unique_ptr<RenderableShader> RenderableShader::clone() const
	{
		auto ret = std::make_unique<RenderableShader>(mEventManager);
		processSteps([&](const StepRef& step) {
			ret->mSteps.push_back(step);
			ret->mTechnique->addPass(step->getPass());
		});
		return ret;
	}


	RenderableShader& RenderableShader::addStep(const StepRef& step)
	{
		mSteps.push_back(step);
		mTechnique->addPass(step->getPass());
		mEventManager.publish(std::make_unique<ShaderEvent>(ShaderEvent::Operation::Add, shared_from_this(), step.get()));
		return *this;
	}


	RenderableShader& RenderableShader::removeStep(const StepRef& step)
	{
		mEventManager.publish(std::make_unique<ShaderEvent>(ShaderEvent::Operation::Remove, shared_from_this(), step.get()));
		mTechnique->removePass(step->getPass());
		mSteps.erase(std::remove(mSteps.begin(), mSteps.end(), step), mSteps.end());
		return *this;
	}

}
