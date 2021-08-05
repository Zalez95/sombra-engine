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


	void IViewProjectionUpdater::removeRenderable(graphics::Renderable& renderable)
	{
		SOMBRA_DEBUG_LOG << "Removing " << &renderable;

		auto itRenderable = mRenderableTechniques.find(&renderable);
		if (itRenderable != mRenderableTechniques.end()) {
			for (std::size_t iTechnique : itRenderable->second) {
				// Remove the Technique if it has no users
				if (--mTechniquesData[iTechnique].userCount == 0) {
					removeTechnique(iTechnique);
				}
			}
		}
	}


	void IViewProjectionUpdater::addRenderableTechnique(graphics::Renderable& renderable, const TechniqueSPtr& technique)
	{
		SOMBRA_DEBUG_LOG << "Adding Technique " << &technique << " to Renderable " << &renderable;

		bool newRenderable = false, newTechnique = false, passAdded = false;

		// Add renderable if it wasn't already
		auto itRenderable = mRenderableTechniques.find(&renderable);
		if (itRenderable == mRenderableTechniques.end()) {
			itRenderable = mRenderableTechniques.emplace(&renderable, IndexVector()).first;
			newRenderable = true;
		}

		// Add technique if it wasn't already
		auto itTechnique = std::find_if(mTechniquesData.begin(), mTechniquesData.end(), [&](const auto& tData) {
			return tData.technique == technique;
		});
		if (itTechnique == mTechniquesData.end()) {
			itTechnique = mTechniquesData.emplace();
			itTechnique->technique = technique;
			newTechnique = true;

			technique->processPasses([&](const PassSPtr& pass) {
				if (shouldAddUniforms(pass)) {
					addPass(itTechnique.getIndex(), pass);
					passAdded = true;
				}
			});
		}

		// Map the renderable with th technique
		itTechnique->userCount++;
		itRenderable->second.push_back(itTechnique.getIndex());

		// Remove the new data shouldAddUniforms failed for every pass
		if (newTechnique && !passAdded) {
			removeRenderableTechnique(renderable, technique);
			if (newRenderable) {
				removeRenderable(renderable);
			}
		}
	}


	void IViewProjectionUpdater::removeRenderableTechnique(
		graphics::Renderable& renderable, const TechniqueSPtr& technique
	) {
		SOMBRA_DEBUG_LOG << "Removing Technique " << &technique << " from Renderable " << &renderable;

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

				// Remove the Technique if it has no users
				if (--itTechnique->userCount == 0) {
					removeTechnique(itTechnique.getIndex());
				}
			}
		}
	}


	void IViewProjectionUpdater::onAddTechniquePass(const TechniqueSPtr& technique, const PassSPtr& pass)
	{
		SOMBRA_DEBUG_LOG << "Adding Pass " << &pass << " to Technique " << &technique;

		auto itTechnique = std::find_if(mTechniquesData.begin(), mTechniquesData.end(), [&](const auto& tData) {
			return tData.technique == technique;
		});
		if (itTechnique != mTechniquesData.end()) {
			if (shouldAddUniforms(pass)) {
				addPass(itTechnique.getIndex(), pass);
			}
		}
	}


	void IViewProjectionUpdater::onRemoveTechniquePass(const TechniqueSPtr& technique, const PassSPtr& pass)
	{
		SOMBRA_DEBUG_LOG << "Removing Pass " << &pass << " from Technique " << &technique;

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
	}


	void IViewProjectionUpdater::update(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		SOMBRA_DEBUG_LOG << "Updating the Passes uniforms";

		for (PassData& passData : mPassesData) {
			passData.viewMatrix->setValue(viewMatrix);
			passData.projectionMatrix->setValue(projectionMatrix);
		}

		SOMBRA_DEBUG_LOG << "Update end";
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
