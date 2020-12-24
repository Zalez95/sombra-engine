#include "se/utils/Log.h"
#include "se/graphics/Renderer.h"
#include "se/graphics/Technique.h"
#include "se/graphics/core/Program.h"
#include "se/app/graphics/IViewProjectionUpdater.h"
#include "se/app/Application.h"
#include "se/app/EntityDatabase.h"
#include "se/app/TransformsComponent.h"

namespace se::app {

	IViewProjectionUpdater::IViewProjectionUpdater(
		const std::string& viewMatUniformName, const std::string& projectionMatUniformName
	) : mViewMatUniformName(viewMatUniformName), mProjectionMatUniformName(projectionMatUniformName) {}


	void IViewProjectionUpdater::addRenderable(graphics::Renderable& renderable)
	{
		glm::mat4 viewMatrix = getViewMatrix();
		glm::mat4 projectionMatrix = getProjectionMatrix();

		auto& passDataIndices = mRenderablePasses[&renderable];
		renderable.processTechniques([&](auto technique) { technique->processPasses([&](auto pass) {
			if (shouldAddUniforms(pass)) {
				auto it = std::find_if(mPassesData.begin(), mPassesData.end(), [&](const auto& passData) { return passData.pass == pass; });
				if (it == mPassesData.end()) {
					std::shared_ptr<graphics::Program> program;
					pass->processBindables([&](const auto& bindable) {
						if (auto tmp = std::dynamic_pointer_cast<graphics::Program>(bindable)) {
							program = tmp;
						}
					});

					if (program) {
						it = mPassesData.emplace();
						it->userCount++;
						it->pass = pass;
						it->viewMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>(
							mViewMatUniformName.c_str(), *program, viewMatrix
						);
						it->projectionMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>(
							mProjectionMatUniformName.c_str(), *program, projectionMatrix
						);

						pass->addBindable(it->viewMatrix)
							.addBindable(it->projectionMatrix);
						passDataIndices.push_back(it.getIndex());
					}
					else {
						SOMBRA_WARN_LOG << "Renderable " << &renderable << "has a Pass " << pass << " with no program";
					}
				}
				else {
					it->userCount++;
					passDataIndices.push_back(it.getIndex());
				}
			}
		}); });
	}


	void IViewProjectionUpdater::removeRenderable(graphics::Renderable& renderable)
	{
		auto it = mRenderablePasses.find(&renderable);
		if (it != mRenderablePasses.end()) {
			for (auto iPass : it->second) {
				mPassesData[iPass].userCount--;
				if (mPassesData[iPass].userCount == 0) {
					mPassesData.erase(mPassesData.begin().setIndex(iPass));
				}
			}

			mRenderablePasses.erase(it);

			SOMBRA_INFO_LOG << "Renderable " << &renderable << " removed successfully";
		}
		else {
			SOMBRA_WARN_LOG << "Renderable " << &renderable << " wan't removed";
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

}
