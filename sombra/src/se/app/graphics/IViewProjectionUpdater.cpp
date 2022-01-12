#include "se/utils/Log.h"
#include "se/utils/ThreadPool.h"
#include "se/graphics/Renderer.h"
#include "se/app/graphics/TypeRefs.h"
#include "se/app/ECS.h"
#include "IViewProjectionUpdater.h"

namespace se::app {

	IViewProjectionUpdater::IViewProjectionUpdater(
		graphics::Context& context, const char* viewMatUniformName, const char* projectionMatUniformName
	) : mContext(context),
		mViewMatUniformName(viewMatUniformName), mProjectionMatUniformName(projectionMatUniformName) {}


	void IViewProjectionUpdater::addRenderableTechnique(graphics::Renderable& renderable, const TechniqueSPtr& technique)
	{
		SOMBRA_DEBUG_LOG << "Adding Technique " << &technique << " to Renderable " << &renderable;
		std::scoped_lock lck(mMutex);

		// Add renderable if it wasn't already
		auto itRenderable = mRenderableTechniques.find(&renderable);
		if (itRenderable == mRenderableTechniques.end()) {
			itRenderable = mRenderableTechniques.emplace(&renderable, IndexVector()).first;
		}

		// Add technique if it wasn't already
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

		// Map the renderable with the technique
		itTechnique->userCount++;
		itRenderable->second.push_back(itTechnique.getIndex());

		SOMBRA_DEBUG_LOG << "Added Technique " << &technique << " to Renderable " << &renderable;
	}


	void IViewProjectionUpdater::removeRenderableTechnique(
		graphics::Renderable& renderable, const TechniqueSPtr& technique
	) {
		SOMBRA_DEBUG_LOG << "Removing Technique " << &technique << " from Renderable " << &renderable;
		std::scoped_lock lck(mMutex);

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

			if (itRenderable->second.empty()) {
				mRenderableTechniques.erase(itRenderable);
			}
		}

		SOMBRA_DEBUG_LOG << "Removed Technique " << &technique << " from Renderable " << &renderable;
	}


	void IViewProjectionUpdater::onAddTechniquePass(const TechniqueSPtr& technique, const PassSPtr& pass)
	{
		SOMBRA_DEBUG_LOG << "Adding Pass " << &pass << " to Technique " << &technique;
		std::scoped_lock lck(mMutex);

		auto itTechnique = std::find_if(mTechniquesData.begin(), mTechniquesData.end(), [&](const auto& tData) {
			return tData.technique == technique;
		});
		if (itTechnique != mTechniquesData.end()) {
			if (shouldAddUniforms(pass)) {
				addPass(itTechnique.getIndex(), pass);
			}
		}

		SOMBRA_DEBUG_LOG << "Added Pass " << &pass << " to Technique " << &technique;
	}


	void IViewProjectionUpdater::onRemoveTechniquePass(const TechniqueSPtr& technique, const PassSPtr& pass)
	{
		SOMBRA_DEBUG_LOG << "Removing Pass " << &pass << " from Technique " << &technique;
		std::scoped_lock lck(mMutex);

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

		SOMBRA_DEBUG_LOG << "Removed Pass " << &pass << " from Technique " << &technique;
	}


	void IViewProjectionUpdater::updateUniformsDeferred(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		SOMBRA_DEBUG_LOG << "Deferred update of the Passes uniforms";
		processNewUniforms();

		std::scoped_lock lck(mMutex);
		for (PassData& passData : mPassesData) {
			if (passData.viewMatrix) {
				passData.viewMatrix.edit([=](auto& uniform) { uniform.setValue(viewMatrix); });
			}
			if (passData.projectionMatrix) {
				passData.projectionMatrix.edit([=](auto& uniform) { uniform.setValue(projectionMatrix); });
			}
		}

		SOMBRA_DEBUG_LOG << "Deferred update end";
	}


	void IViewProjectionUpdater::updateUniforms(
		graphics::Context::Query& q, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix
	) {
		SOMBRA_DEBUG_LOG << "Updating the Passes uniforms";
		processNewUniforms();

		std::scoped_lock lck(mMutex);
		for (PassData& passData : mPassesData) {
			if (passData.viewMatrix) {
				q.getTBindable(passData.viewMatrix)->setValue(viewMatrix);
			}
			if (passData.projectionMatrix) {
				q.getTBindable(passData.projectionMatrix)->setValue(projectionMatrix);
			}
		}

		SOMBRA_DEBUG_LOG << "Update end";
	}

// Private functions
	void IViewProjectionUpdater::addPass(std::size_t iTechnique, const PassSPtr& pass)
	{
		auto itPass = std::find_if(mPassesData.begin(), mPassesData.end(), [&](const auto& pData) {
			return pData.pass == pass;
		});
		if (itPass != mPassesData.end()) {
			itPass->userCount++;
			mTechniquesData[iTechnique].passIndices.push_back(itPass.getIndex());
			return;
		}

		itPass = mPassesData.emplace(PassData{ 1, pass, {}, {} });
		mTechniquesData[iTechnique].passIndices.push_back(itPass.getIndex());

		auto pView = std::make_shared<std::promise<UniformVVRef<glm::mat4>>>();
		mNewUniforms.push(NewUniform{ 0, pass, pView->get_future() });
		auto pProj = std::make_shared<std::promise<UniformVVRef<glm::mat4>>>();
		mNewUniforms.push(NewUniform{ 1, pass, pProj->get_future() });

		mContext.execute([=](graphics::Context::Query& q) {
			UniformVVRef<glm::mat4> viewMatrix, projectionMatrix;

			pass->processBindables([&](const auto& bindable) {
				if (auto uMat = UniformVVRef<glm::mat4>::from(bindable)) {
					if (q.getTBindable(uMat)->getName() == mViewMatUniformName) {
						viewMatrix = uMat;
					}
					else if (q.getTBindable(uMat)->getName() == mProjectionMatUniformName) {
						projectionMatrix = uMat;
					}
				}
			});

			pView->set_value(viewMatrix);
			pProj->set_value(projectionMatrix);
		});
	}


	void IViewProjectionUpdater::removePass(std::size_t iTechnique, std::size_t iPass)
	{
		auto& passIndices = mTechniquesData[iTechnique].passIndices;
		passIndices.erase(
			std::remove(passIndices.begin(), passIndices.end(), iPass),
			passIndices.end()
		);

		if (--mPassesData[iPass].userCount == 0) {
			mPassesData[iPass].pass->removeBindable(mPassesData[iPass].viewMatrix);
			mPassesData[iPass].pass->removeBindable(mPassesData[iPass].projectionMatrix);
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


	void IViewProjectionUpdater::processNewUniforms()
	{
		std::unique_lock lock(mMutex);

		std::queue<NewUniform> nextNewUniforms;
		while (!mNewUniforms.empty()) {
			NewUniform newUniform = std::move(mNewUniforms.front());
			mNewUniforms.pop();
			lock.unlock();

			if (utils::is_ready(newUniform.uniformMat)) {
				auto uniformMat = newUniform.uniformMat.get();
				if (uniformMat) {
					lock.lock();
					auto itPass = std::find_if(mPassesData.begin(), mPassesData.end(), [&](const PassData& pd) { return pd.pass == newUniform.pass; });
					if (itPass != mPassesData.end()) {
						if ((newUniform.type & 0x1) > 0) {
							itPass->projectionMatrix = uniformMat;
						}
						else {
							itPass->viewMatrix = uniformMat;
						}

						if ((newUniform.type & 0x2) > 0) {
							newUniform.pass->addBindable(uniformMat);
						}
					}
					lock.unlock();
				}
				else if ((newUniform.type & 0x2) == 0) {
					ProgramRef program;
					newUniform.pass->processBindables([&](const auto& bindable) {
						if (auto prog = ProgramRef::from(bindable)) {
							program = prog;
						}
					});

					if (program) {
						SOMBRA_DEBUG_LOG << "Creating new uniform " << (newUniform.type & 0x1) << " for pass " << newUniform.pass;

						auto p = std::make_shared<std::promise<UniformVVRef<glm::mat4>>>();
						newUniform.type |= 0x2;
						newUniform.uniformMat = p->get_future();

						const auto& uniformName = ((newUniform.type & 0x1) > 0)? mProjectionMatUniformName : mViewMatUniformName;
						uniformMat = mContext.create<graphics::UniformVariableValue<glm::mat4>>(uniformName);
						uniformMat.qedit([=](auto& q, auto& uniform) {
							if (uniform.load(*q.getTBindable(program))) {
								p->set_value(uniformMat);
							}
							else {
								p->set_value(UniformVVRef<glm::mat4>());
							}
						});

						lock.lock();
						nextNewUniforms.push(std::move(newUniform));
						lock.unlock();
					}
					else {
						SOMBRA_WARN_LOG << "Trying to add a Pass " << newUniform.pass << " with no program";
					}
				}
				else {
					SOMBRA_WARN_LOG << "Uniform " << (newUniform.type & 0x1) << " couldn't be added to pass " << newUniform.pass;
				}
			}
			else {
				nextNewUniforms.push(std::move(newUniform));
			}

			lock.lock();
		}

		std::swap(mNewUniforms, nextNewUniforms);
	}

}
