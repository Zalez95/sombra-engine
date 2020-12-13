#include "se/utils/Log.h"
#include "se/graphics/Technique.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/graphics/core/Program.h"
#include "se/app/RMeshSystem.h"
#include "se/app/Application.h"
#include "se/app/MeshComponent.h"
#include "se/app/EntityDatabase.h"
#include "se/app/TransformsComponent.h"

namespace se::app {

	RMeshSystem::RMeshSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application)
	{
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask().set<MeshComponent>().set<SkinComponent>());
	}


	RMeshSystem::~RMeshSystem()
	{
		mEntityDatabase.removeSystem(this);
	}


	void RMeshSystem::onNewEntity(Entity entity)
	{
		auto [transforms, mesh, skin] = mEntityDatabase.getComponents<TransformsComponent, MeshComponent, SkinComponent>(entity);
		if (!mesh) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as Mesh";
			return;
		}

		glm::mat4 modelMatrix(1.0f);
		if (transforms) {
			glm::mat4 translation	= glm::translate(glm::mat4(1.0f), transforms->position);
			glm::mat4 rotation		= glm::mat4_cast(transforms->orientation);
			glm::mat4 scale			= glm::scale(glm::mat4(1.0f), transforms->scale);
			modelMatrix = translation * rotation * scale;
		}

		auto& entityUniforms = mEntityUniforms[entity];
		entityUniforms.clear();
		for (auto& rMesh : mesh->rMeshes) {
			rMesh.setModelMatrix(modelMatrix);
			rMesh.processTechniques([&, skin = skin](auto technique) { technique->processPasses([&](auto pass) {
				std::shared_ptr<graphics::Program> program;
				pass->processBindables([&](const auto& bindable) {
					if (auto tmp = std::dynamic_pointer_cast<graphics::Program>(bindable)) {
						program = tmp;
					}
				});

				if (program) {
					auto& meshUniforms = entityUniforms.emplace_back();
					meshUniforms.pass = pass;
					meshUniforms.modelMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", *program, modelMatrix);
					rMesh.addPassBindable(pass.get(), meshUniforms.modelMatrix);

					if (skin) {
						auto jointMatrices = skin->calculateJointMatrices(modelMatrix);
						std::size_t numJoints = std::min(jointMatrices.size(), static_cast<std::size_t>(Skin::kMaxJoints));
						meshUniforms.jointMatrices = std::make_shared<graphics::UniformVariableValueVector<glm::mat4, Skin::kMaxJoints>>(
							"uJointMatrices", *program, jointMatrices.data(), numJoints
						);
						rMesh.addPassBindable(pass.get(), meshUniforms.jointMatrices);
					}
				}
				else {
					SOMBRA_WARN_LOG << "RenderableMesh has a Pass " << pass << " with no program";
				}
			}); });

			mApplication.getExternalTools().graphicsEngine->addRenderable(&rMesh);
		}

		SOMBRA_INFO_LOG << "Entity " << entity << " with MeshComponent " << mesh << " added successfully";
	}


	void RMeshSystem::onRemoveEntity(Entity entity)
	{
		auto [mesh] = mEntityDatabase.getComponents<MeshComponent>(entity);
		if (!mesh) {
			SOMBRA_INFO_LOG << "Mesh Entity " << entity << " couldn't removed";
			return;
		}

		auto it = mEntityUniforms.find(entity);
		if (it != mEntityUniforms.end()) {
			for (auto& rMesh : mesh->rMeshes) {
				mApplication.getExternalTools().graphicsEngine->removeRenderable(&rMesh);

				for (auto& uniforms : it->second) {
					rMesh.removePassBindable(uniforms.pass.get(), uniforms.modelMatrix);
					rMesh.removePassBindable(uniforms.pass.get(), uniforms.jointMatrices);
				}
			}

			mEntityUniforms.erase(it);
		}

		SOMBRA_INFO_LOG << "Mesh Entity " << entity << " removed successfully";
	}


	void RMeshSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Meshes";

		for (auto [entity, entityUniforms] : mEntityUniforms) {
			auto [transforms, mesh, skin] = mEntityDatabase.getComponents<TransformsComponent, MeshComponent, SkinComponent>(entity);
			if (transforms && transforms->updated.any()) {
				glm::mat4 translation	= glm::translate(glm::mat4(1.0f), transforms->position);
				glm::mat4 rotation		= glm::mat4_cast(transforms->orientation);
				glm::mat4 scale			= glm::scale(glm::mat4(1.0f), transforms->scale);
				glm::mat4 modelMatrix	= translation * rotation * scale;

				for (auto& rMesh : mesh->rMeshes) {
					rMesh.setModelMatrix(modelMatrix);
				}

				for (auto& meshUniforms : entityUniforms) {
					meshUniforms.modelMatrix->setValue(modelMatrix);
					if (skin) {
						auto jointMatrices = skin->calculateJointMatrices(modelMatrix);
						std::size_t numJoints = std::min(jointMatrices.size(), static_cast<std::size_t>(Skin::kMaxJoints));
						meshUniforms.jointMatrices->setValue(jointMatrices.data(), numJoints);
					}
				}
			}
		}

		SOMBRA_INFO_LOG << "Update end";
	}

}
