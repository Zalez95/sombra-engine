#include "se/utils/Log.h"
#include "se/graphics/Renderer.h"
#include "se/graphics/Technique.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/3D/RenderableTerrain.h"
#include "se/app/IVPSystem.h"
#include "se/app/Application.h"
#include "se/app/MeshComponent.h"
#include "se/app/EntityDatabase.h"
#include "se/app/TransformsComponent.h"

namespace se::app {

	IVPSystem::IVPSystem(
		Application& application,
		const std::string& viewMatUniform, const std::string& projectionMatUniform
	) : ISystem(application.getEntityDatabase()), mApplication(application),
		mViewMatUniform(viewMatUniform), mProjectionMatUniform(projectionMatUniform)
	{
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask()
			.set<MeshComponent>()
			.set<graphics::RenderableTerrain>()
		);
	}


	IVPSystem::~IVPSystem()
	{
		mEntityDatabase.removeSystem(this);
	}


	void IVPSystem::onNewEntity(Entity entity)
	{
		auto [transforms, mesh, rTerrain] = mEntityDatabase.getComponents<
			TransformsComponent, MeshComponent, graphics::RenderableTerrain
		>(entity);

		std::vector<std::size_t> passDataIndices;
		if (mesh) {
			for (auto& rMesh : mesh->rMeshes) {
				processPasses(rMesh, passDataIndices);
			}
		}
		if (rTerrain) {
			processPasses(*rTerrain, passDataIndices);
		}
		mEntityPasses.emplace(entity, std::move(passDataIndices));
	}


	void IVPSystem::onRemoveEntity(Entity entity)
	{
		auto it = mEntityPasses.find(entity);
		if (it != mEntityPasses.end()) {
			for (auto iPass : it->second) {
				mPassesData[iPass].userCount--;
				if (mPassesData[iPass].userCount == 0) {
					mPassesData.erase(mPassesData.begin().setIndex(iPass));
				}
			}

			mEntityPasses.erase(it);

			SOMBRA_INFO_LOG << "Entity " << entity << " removed successfully";
		}
		else {
			SOMBRA_WARN_LOG << "Entity " << entity << " wan't removed";
		}
	}


	void IVPSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Passes uniforms";

		for (auto& passData : mPassesData) {
			passData.viewMatrix->setValue(getViewMatrix());
			passData.projectionMatrix->setValue(getProjectionMatrix());
		}

		SOMBRA_INFO_LOG << "Update end";
	}

// Private functions
	void IVPSystem::processPasses(graphics::Renderable3D& renderable, std::vector<std::size_t>& output)
	{
		glm::mat4 viewMatrix = getViewMatrix();
		glm::mat4 projectionMatrix = getProjectionMatrix();

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
							mViewMatUniform.c_str(), *program, viewMatrix
						);
						it->projectionMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>(
							mProjectionMatUniform.c_str(), *program, projectionMatrix
						);

						pass->addBindable(it->viewMatrix)
							.addBindable(it->projectionMatrix);
						output.push_back(it.getIndex());
					}
					else {
						SOMBRA_WARN_LOG << "Renderable3D has a Pass " << pass << " with no program";
					}
				}
				else {
					it->userCount++;
					output.push_back(it.getIndex());
				}
			}
		}); });
	}

}
