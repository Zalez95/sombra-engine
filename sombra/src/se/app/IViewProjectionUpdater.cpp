#include "se/utils/Log.h"
#include "se/graphics/Renderer.h"
#include "se/graphics/Technique.h"
#include "se/graphics/core/Program.h"
#include "se/app/IViewProjectionUpdater.h"
#include "se/app/Application.h"
#include "se/app/EntityDatabase.h"
#include "se/app/TransformsComponent.h"

namespace se::app {

	IViewProjectionUpdater::IViewProjectionUpdater(
		const std::string& viewMatUniformName, const std::string& projectionMatUniformName
	) : mViewMatUniformName(viewMatUniformName), mProjectionMatUniformName(projectionMatUniformName) {}


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
				shader->getTechnique()->processPasses([&](const PassSPtr& pass) {
					if (shouldAddUniforms(pass)) {
						addPass(itShader.getIndex(), pass);
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


	void IViewProjectionUpdater::onAddShaderPass(const RenderableShaderSPtr& shader, const PassSPtr& pass)
	{
		auto itShader = std::find_if(mShadersData.begin(), mShadersData.end(), [&](const auto& sData) {
			return sData.shader == shader;
		});
		if (itShader != mShadersData.end()) {
			if (shouldAddUniforms(pass)) {
				addPass(itShader.getIndex(), pass);
			}
		}
		else {
			SOMBRA_WARN_LOG << "Shader " << shader << " not found";
		}
	}


	void IViewProjectionUpdater::onRemoveShaderPass(const RenderableShaderSPtr& shader, const PassSPtr& pass)
	{
		auto itShader = std::find_if(mShadersData.begin(), mShadersData.end(), [&](const auto& sData) {
			return sData.shader == shader;
		});
		if (itShader != mShadersData.end()) {
			auto itPass = std::find_if(mPassesData.begin(), mPassesData.end(), [&](const auto& pData) {
				return pData.pass == pass;
			});
			if (itPass != mPassesData.end()) {
				removePass(itShader.getIndex(), itPass.getIndex());
			}
		}
		else {
			SOMBRA_WARN_LOG << "Shader " << shader << " not found";
		}
	}


	void IViewProjectionUpdater::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Passes uniforms";

		auto viewMatrix = getViewMatrix();
		auto projectionMatrix = getProjectionMatrix();

		for (auto& passData : mPassesData) {
			passData.viewMatrix->setValue(viewMatrix);
			passData.projectionMatrix->setValue(projectionMatrix);
		}

		SOMBRA_INFO_LOG << "Update end";
	}

// Private functions
	void IViewProjectionUpdater::addPass(std::size_t iShader, const PassSPtr& pass)
	{
		auto itPass = std::find_if(mPassesData.begin(), mPassesData.end(), [&](const auto& pData) {
			return pData.pass == pass;
		});
		if (itPass == mPassesData.end()) {
			std::shared_ptr<graphics::Program> program;
			pass->processBindables([&](const auto& bindable) {
				if (auto tmp = std::dynamic_pointer_cast<graphics::Program>(bindable)) {
					program = tmp;
				}
			});

			if (!program) {
				SOMBRA_WARN_LOG << "Trying to add a Pass " << pass << " with no program";
				return;
			}

			itPass = mPassesData.emplace();
			itPass->pass = pass;
			itPass->viewMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>(
				mViewMatUniformName.c_str(), program
			);
			itPass->projectionMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>(
				mProjectionMatUniformName.c_str(), program
			);

			if (itPass->viewMatrix->found()) {
				pass->addBindable(itPass->viewMatrix);
			}
			if (itPass->projectionMatrix->found()) {
				pass->addBindable(itPass->projectionMatrix);
			}
		}

		itPass->userCount++;
		mShadersData[iShader].passIndices.push_back(itPass.getIndex());
	}


	void IViewProjectionUpdater::removePass(std::size_t iShader, std::size_t iPass)
	{
		auto& passIndices = mShadersData[iShader].passIndices;
		passIndices.erase(
			std::remove(passIndices.begin(), passIndices.end(), iPass),
			passIndices.end()
		);

		if (--mPassesData[iPass].userCount == 0) {
			if (mPassesData[iPass].viewMatrix->found()) {
				mPassesData[iPass].pass->removeBindable(mPassesData[iPass].viewMatrix);
			}
			if (mPassesData[iPass].projectionMatrix->found()) {
				mPassesData[iPass].pass->removeBindable(mPassesData[iPass].projectionMatrix);
			}

			mPassesData.erase(mPassesData.begin().setIndex(iPass));
		}
	}


	void IViewProjectionUpdater::removeShader(std::size_t iShader)
	{
		for (std::size_t iPass : mShadersData[iShader].passIndices) {
			removePass(iShader, iPass);
		}
	}

}
