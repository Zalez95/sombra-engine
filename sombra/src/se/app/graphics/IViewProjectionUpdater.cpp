#include "se/utils/Log.h"
#include "se/graphics/Renderer.h"
#include "se/graphics/Technique.h"
#include "se/graphics/core/Program.h"
#include "se/app/Application.h"
#include "se/app/ECS.h"
#include "se/app/TransformsComponent.h"
#include "IViewProjectionUpdater.h"

namespace se::app {

	IViewProjectionUpdater::IViewProjectionUpdater(const char* viewMatUniformName, const char* projectionMatUniformName) :
		mViewMatUniformName(viewMatUniformName), mProjectionMatUniformName(projectionMatUniformName) {}


	void IViewProjectionUpdater::addRenderable(graphics::Renderable& renderable)
	{
		mRenderableShaders.emplace(&renderable, IndexVector());
	}


	void IViewProjectionUpdater::removeRenderable(graphics::Renderable& renderable)
	{
		auto itRenderable = mRenderableShaders.find(&renderable);
		if (itRenderable != mRenderableShaders.end()) {
			for (std::size_t iShader : itRenderable->second) {
				if (--mShadersData[iShader].userCount == 0) {
					removeShader(iShader);
				}
			}
		}
		else {
			SOMBRA_WARN_LOG << "Renderable " << &renderable << " not found";
		}
	}


	void IViewProjectionUpdater::addRenderableShader(
		graphics::Renderable& renderable, const RenderableShaderSPtr& shader
	) {
		auto itRenderable = mRenderableShaders.find(&renderable);
		if (itRenderable != mRenderableShaders.end()) {
			auto itShader = std::find_if(mShadersData.begin(), mShadersData.end(), [&](const auto& sData) {
				return sData.shader == shader;
			});
			if (itShader == mShadersData.end()) {
				itShader = mShadersData.emplace();
				itShader->shader = shader;
				shader->processSteps([&](const auto& step) {
					if (shouldAddUniforms(step.get())) {
						addStep(itShader.getIndex(), step.get());
					}
				});
			}

			itShader->userCount++;
			itRenderable->second.push_back(itShader.getIndex());
		}
		else {
			SOMBRA_WARN_LOG << "Renderable " << &renderable << " not found";
		}
	}


	void IViewProjectionUpdater::removeRenderableShader(
		graphics::Renderable& renderable, const RenderableShaderSPtr& shader
	) {
		auto itRenderable = mRenderableShaders.find(&renderable);
		if (itRenderable != mRenderableShaders.end()) {
			auto itShader = std::find_if(mShadersData.begin(), mShadersData.end(), [&](const auto& sData) {
				return sData.shader == shader;
			});
			if (itShader != mShadersData.end()) {
				itRenderable->second.erase(
					std::remove(itRenderable->second.begin(), itRenderable->second.end(), itShader.getIndex()),
					itRenderable->second.end()
				);
				if (--itShader->userCount == 0) {
					removeShader(itShader.getIndex());
				}
			}
			else {
				SOMBRA_WARN_LOG << "Shader " << shader << " not found";
			}
		}
		else {
			SOMBRA_WARN_LOG << "Renderable " << &renderable << " not found";
		}
	}


	void IViewProjectionUpdater::onAddShaderStep(const RenderableShaderSPtr& shader, const RenderableShaderStepSPtr& step)
	{
		auto itShader = std::find_if(mShadersData.begin(), mShadersData.end(), [&](const auto& sData) {
			return sData.shader == shader;
		});
		if (itShader != mShadersData.end()) {
			if (shouldAddUniforms(step)) {
				addStep(itShader.getIndex(), step);
			}
		}
		else {
			SOMBRA_WARN_LOG << "Shader " << shader << " not found";
		}
	}


	void IViewProjectionUpdater::onRemoveShaderStep(const RenderableShaderSPtr& shader, const RenderableShaderStepSPtr& step)
	{
		auto itShader = std::find_if(mShadersData.begin(), mShadersData.end(), [&](const auto& sData) {
			return sData.shader == shader;
		});
		if (itShader != mShadersData.end()) {
			auto itStep = std::find_if(mStepsData.begin(), mStepsData.end(), [&](const auto& pData) {
				return pData.step == step;
			});
			if (itStep != mStepsData.end()) {
				removeStep(itShader.getIndex(), itStep.getIndex());
			}
		}
		else {
			SOMBRA_WARN_LOG << "Shader " << shader << " not found";
		}
	}


	void IViewProjectionUpdater::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Steps uniforms";

		auto viewMatrix = getViewMatrix();
		auto projectionMatrix = getProjectionMatrix();

		for (auto& stepData : mStepsData) {
			stepData.viewMatrix->setValue(viewMatrix);
			stepData.projectionMatrix->setValue(projectionMatrix);
		}

		SOMBRA_INFO_LOG << "Update end";
	}

// Private functions
	void IViewProjectionUpdater::addStep(std::size_t iShader, const RenderableShaderStepSPtr& step)
	{
		auto itStep = std::find_if(mStepsData.begin(), mStepsData.end(), [&](const auto& pData) {
			return pData.step == step;
		});
		if (itStep == mStepsData.end()) {
			std::shared_ptr<graphics::Program> program;
			Mat4UniformSPtr viewMatrix, projectionMatrix;
			step->processBindables([&](const auto& bindable) {
				if (auto prog = std::dynamic_pointer_cast<graphics::Program>(bindable)) {
					program = prog;
				}
				else if (auto uMat = std::dynamic_pointer_cast<Mat4Uniform>(bindable)) {
					if (uMat->getName() == mViewMatUniformName) {
						viewMatrix = uMat;
					}
					else if (uMat->getName() == mProjectionMatUniformName) {
						projectionMatrix = uMat;
					}
				}
			});

			if (!program) {
				SOMBRA_WARN_LOG << "Trying to add a Step " << step << " with no program";
				return;
			}

			if (!viewMatrix) {
				viewMatrix = std::make_shared<Mat4Uniform>(mViewMatUniformName.c_str(), program);
				if (viewMatrix->found()) {
					step->addBindable(viewMatrix);
				}
			}

			if (!projectionMatrix) {
				projectionMatrix = std::make_shared<Mat4Uniform>(mProjectionMatUniformName.c_str(), program);
				if (projectionMatrix->found()) {
					step->addBindable(projectionMatrix);
				}
			}

			itStep = mStepsData.emplace(StepData{ 0, step, viewMatrix, projectionMatrix });
		}

		itStep->userCount++;
		mShadersData[iShader].stepIndices.push_back(itStep.getIndex());
	}


	void IViewProjectionUpdater::removeStep(std::size_t iShader, std::size_t iStep)
	{
		auto& stepIndices = mShadersData[iShader].stepIndices;
		stepIndices.erase(
			std::remove(stepIndices.begin(), stepIndices.end(), iStep),
			stepIndices.end()
		);

		if (--mStepsData[iStep].userCount == 0) {
			if (mStepsData[iStep].viewMatrix->found()) {
				mStepsData[iStep].step->removeBindable(mStepsData[iStep].viewMatrix);
			}
			if (mStepsData[iStep].projectionMatrix->found()) {
				mStepsData[iStep].step->removeBindable(mStepsData[iStep].projectionMatrix);
			}

			mStepsData.erase(mStepsData.begin().setIndex(iStep));
		}
	}


	void IViewProjectionUpdater::removeShader(std::size_t iShader)
	{
		for (std::size_t iStep : mShadersData[iShader].stepIndices) {
			removeStep(iShader, iStep);
		}
		mShadersData.erase( mShadersData.begin().setIndex(iShader) );
	}

}
