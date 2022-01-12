#include "se/app/events/ShaderEvent.h"
#include "se/app/graphics/RenderableShader.h"

namespace se::app {

	template <>
	RenderableShaderStep& RenderableShaderStep::addResource<ProgramRef>(
		const Repository::ResourceRef<ProgramRef>& resource, bool addResource
	) {
		mProgramResources.push_back(resource);
		if (addResource) {
			addBindable(*resource);
		}
		return *this;
	}


	template <>
	RenderableShaderStep& RenderableShaderStep::addResource<TextureRef>(
		const Repository::ResourceRef<TextureRef>& resource, bool addResource
	) {
		mTextureResources.push_back(resource);
		if (addResource) {
			addBindable(*resource);
		}
		return *this;
	}


	std::unique_ptr<RenderableShaderStep> RenderableShaderStep::clone() const
	{
		auto ret = std::make_unique<RenderableShaderStep>(mPass->getRenderer());
		for (const auto& program : mProgramResources) {
			ret->addResource(program);
		}
		for (const auto& texture : mTextureResources) {
			ret->addResource(texture);
		}
		processBindables([&](const graphics::Context::BindableRef& bindable) {
			if (!TextureRef::from(bindable) && !ProgramRef::from(bindable)) {
				ret->addBindable(bindable.clone());
			}
		});
		return ret;
	}


	template <>
	RenderableShaderStep& RenderableShaderStep::removeResource<ProgramRef>(
		const Repository::ResourceRef<ProgramRef>& resource, bool removeResource
	) {
		mProgramResources.erase(
			std::remove(mProgramResources.begin(), mProgramResources.end(), resource),
			mProgramResources.end()
		);
		if (removeResource) {
			removeBindable(*resource);
		}
		return *this;
	}


	template <>
	RenderableShaderStep& RenderableShaderStep::removeResource<TextureRef>(
		const Repository::ResourceRef<TextureRef>& resource, bool removeResource
	) {
		mTextureResources.erase(
			std::remove(mTextureResources.begin(), mTextureResources.end(), resource),
			mTextureResources.end()
		);
		if (removeResource) {
			removeBindable(*resource);
		}
		return *this;
	}


	RenderableShaderStep& RenderableShaderStep::addBindable(const graphics::Context::BindableRef& bindable)
	{
		mPass->addBindable(bindable);
		return *this;
	}


	RenderableShaderStep& RenderableShaderStep::removeBindable(const graphics::Context::BindableRef& bindable)
	{
		mPass->removeBindable(bindable);
		return *this;
	}


	std::unique_ptr<RenderableShader> RenderableShader::clone() const
	{
		auto ret = std::make_unique<RenderableShader>(mEventManager);
		processSteps([&](const StepResource& step) {
			ret->mSteps.push_back(step);
			ret->mTechnique->addPass(step->getPass());
		});
		return ret;
	}


	RenderableShader& RenderableShader::addStep(const StepResource& step)
	{
		mSteps.push_back(step);
		mTechnique->addPass(step->getPass());
		mEventManager.publish(std::make_unique<ShaderEvent>(ShaderEvent::Operation::Add, shared_from_this(), step.get()));
		return *this;
	}


	RenderableShader& RenderableShader::removeStep(const StepResource& step)
	{
		mEventManager.publish(std::make_unique<ShaderEvent>(ShaderEvent::Operation::Remove, shared_from_this(), step.get()));
		mTechnique->removePass(step->getPass());
		mSteps.erase(std::remove(mSteps.begin(), mSteps.end(), step), mSteps.end());
		return *this;
	}

}
