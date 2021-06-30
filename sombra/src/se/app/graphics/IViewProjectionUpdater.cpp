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
		mRenderableTechniques.emplace(&renderable, IndexVector());
	}


	void IViewProjectionUpdater::removeRenderable(graphics::Renderable& renderable)
	{
		auto itRenderable = mRenderableTechniques.find(&renderable);
		if (itRenderable != mRenderableTechniques.end()) {
			for (std::size_t iTechnique : itRenderable->second) {
				if (--mTechniquesData[iTechnique].userCount == 0) {
					removeTechnique(iTechnique);
				}
			}
		}
		else {
			SOMBRA_WARN_LOG << "Renderable " << &renderable << " not found";
		}
	}


	void IViewProjectionUpdater::addRenderableTechnique(graphics::Renderable& renderable, const TechniqueSPtr& technique)
	{
		auto itRenderable = mRenderableTechniques.find(&renderable);
		if (itRenderable != mRenderableTechniques.end()) {
			auto itTechnique = std::find_if(mTechniquesData.begin(), mTechniquesData.end(), [&](const auto& tData) {
				return tData.technique == technique;
			});
			if (itTechnique == mTechniquesData.end()) {
				itTechnique = mTechniquesData.emplace();
				itTechnique->technique = technique;
				technique->processPasses([&](const PassSPtr& pass) {
					if (shouldAddUniforms(pass)) {
						addPass(itTechnique.getIndex(), pass);
					}
				});
			}

			itTechnique->userCount++;
			itRenderable->second.push_back(itTechnique.getIndex());
		}
		else {
			SOMBRA_WARN_LOG << "Renderable " << &renderable << " not found";
		}
	}


	void IViewProjectionUpdater::removeRenderableTechnique(
		graphics::Renderable& renderable, const TechniqueSPtr& technique
	) {
		auto itRenderable = mRenderableTechniques.find(&renderable);
		if (itRenderable != mRenderableTechniques.end()) {
			auto itTechnique = std::find_if(mTechniquesData.begin(), mTechniquesData.end(), [&](const auto& tData) {
				return tData.technique == technique;
			});
			if (itTechnique != mTechniquesData.end()) {
				itRenderable->second.erase(
					std::remove(itRenderable->second.begin(), itRenderable->second.end(), itTechnique.getIndex()),
					itRenderable->second.end()
				);
				if (--itTechnique->userCount == 0) {
					removeTechnique(itTechnique.getIndex());
				}
			}
			else {
				SOMBRA_WARN_LOG << "Technique " << technique << " not found";
			}
		}
		else {
			SOMBRA_WARN_LOG << "Renderable " << &renderable << " not found";
		}
	}


	void IViewProjectionUpdater::onAddTechniquePass(const TechniqueSPtr& technique, const PassSPtr& pass)
	{
		auto itTechnique = std::find_if(mTechniquesData.begin(), mTechniquesData.end(), [&](const auto& tData) {
			return tData.technique == technique;
		});
		if (itTechnique != mTechniquesData.end()) {
			if (shouldAddUniforms(pass)) {
				addPass(itTechnique.getIndex(), pass);
			}
		}
		else {
			SOMBRA_WARN_LOG << "Technique " << technique << " not found";
		}
	}


	void IViewProjectionUpdater::onRemoveTechniquePass(const TechniqueSPtr& technique, const PassSPtr& pass)
	{
		auto itTechnique = std::find_if(mTechniquesData.begin(), mTechniquesData.end(), [&](const auto& tData) {
			return tData.technique == technique;
		});
		if (itTechnique != mTechniquesData.end()) {
			auto itPass = std::find_if(mPassesData.begin(), mPassesData.end(), [&](const auto& pData) {
				return pData.pass == pass;
			});
			if (itPass != mPassesData.end()) {
				removePass(itTechnique.getIndex(), itPass.getIndex());
			}
		}
		else {
			SOMBRA_WARN_LOG << "Technique " << technique << " not found";
		}
	}


	void IViewProjectionUpdater::update(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		SOMBRA_DEBUG_LOG << "Updating the Passes uniforms";

		for (PassData& passData : mPassesData) {
			passData.viewMatrix->setValue(viewMatrix);
			passData.projectionMatrix->setValue(projectionMatrix);
		}

		SOMBRA_INFO_LOG << "Update end";
	}

// Private functions
	void IViewProjectionUpdater::addPass(std::size_t iTechnique, const PassSPtr& pass)
	{
		auto itPass = std::find_if(mPassesData.begin(), mPassesData.end(), [&](const auto& pData) {
			return pData.pass == pass;
		});
		if (itPass == mPassesData.end()) {
			std::shared_ptr<graphics::Program> program;
			Mat4UniformSPtr viewMatrix, projectionMatrix;
			pass->processBindables([&](const auto& bindable) {
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
				SOMBRA_WARN_LOG << "Trying to add a Pass " << pass << " with no program";
				return;
			}

			if (!viewMatrix) {
				viewMatrix = std::make_shared<Mat4Uniform>(mViewMatUniformName.c_str(), program);
				if (viewMatrix->found()) {
					pass->addBindable(viewMatrix);
				}
			}

			if (!projectionMatrix) {
				projectionMatrix = std::make_shared<Mat4Uniform>(mProjectionMatUniformName.c_str(), program);
				if (projectionMatrix->found()) {
					pass->addBindable(projectionMatrix);
				}
			}

			itPass = mPassesData.emplace(PassData{ 0, pass, viewMatrix, projectionMatrix });
		}

		itPass->userCount++;
		mTechniquesData[iTechnique].passIndices.push_back(itPass.getIndex());
	}


	void IViewProjectionUpdater::removePass(std::size_t iTechnique, std::size_t iPass)
	{
		auto& passIndices = mTechniquesData[iTechnique].passIndices;
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


	void IViewProjectionUpdater::removeTechnique(std::size_t iTechnique)
	{
		for (std::size_t iPass : mTechniquesData[iTechnique].passIndices) {
			removePass(iTechnique, iPass);
		}
		mTechniquesData.erase( mTechniquesData.begin().setIndex(iTechnique) );
	}

}
