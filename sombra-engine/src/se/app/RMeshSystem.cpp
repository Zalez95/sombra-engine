#include "se/utils/Log.h"
#include "se/graphics/Technique.h"
#include "se/graphics/Pass.h"
#include "se/app/RMeshSystem.h"
#include "se/app/EntityDatabase.h"
#include "se/app/MeshComponent.h"
#include "se/app/TransformsComponent.h"
#include "se/app/graphics/Skin.h"

namespace se::app {

	RMeshSystem::RMeshSystem(EntityDatabase& entityDatabase, graphics::GraphicsEngine& graphicsEngine, CameraSystem& cameraSystem) :
		ISystem(entityDatabase), mGraphicsEngine(graphicsEngine), mCameraSystem(cameraSystem)
	{
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask().set<MeshComponent>());
	}


	RMeshSystem::~RMeshSystem()
	{
		mEntityDatabase.removeSystem(this);
	}


	void RMeshSystem::onNewEntity(Entity entity)
	{
		auto [transforms, mesh, skin] = mEntityDatabase.getComponents<TransformsComponent, MeshComponent, Skin>(entity);
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

		for (std::size_t i = 0; i < mesh->rMeshes.size(); ++i) {
			auto& rMesh = mesh->rMeshes[i];
			auto& meshUniforms = mRMeshUniforms[std::make_pair(entity, i)];

			rMesh.processTechniques([&, skin = skin](auto technique) { technique->processPasses([&](auto pass) {
				auto it = std::find_if(
					mCameraSystem.mPassesData.begin(), mCameraSystem.mPassesData.end(),
					[&](const auto& passData) { return passData.pass == pass; }
				);
				if (it != mCameraSystem.mPassesData.end()) {
					meshUniforms.modelMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", *it->program, modelMatrix);
					rMesh.addBindable(meshUniforms.modelMatrix);

					if (skin) {
						auto jointMatrices = calculateJointMatrices(*skin, modelMatrix);
						std::size_t numJoints = std::min(jointMatrices.size(), static_cast<std::size_t>(kMaxJoints));
						meshUniforms.jointMatrices = std::make_shared<graphics::UniformVariableValueVector<glm::mat4, kMaxJoints>>(
							"uJointMatrices", *it->program, jointMatrices.data(), numJoints
						);
						rMesh.addBindable(meshUniforms.jointMatrices);
					}
				}
				else {
					SOMBRA_WARN_LOG << "RenderableMesh has a Pass " << pass << " not added to the CameraSystem";
				}
			}); });

			mGraphicsEngine.addRenderable(&rMesh);
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

		for (std::size_t i = 0; i < mesh->rMeshes.size(); ++i) {
			auto it = mRMeshUniforms.find(std::make_pair(entity, i));
			if (it != mRMeshUniforms.end()) {
				mRMeshUniforms.erase(it);
			}

			mGraphicsEngine.removeRenderable(&mesh->rMeshes[i]);
		}

		SOMBRA_INFO_LOG << "Mesh Entity " << entity << " removed successfully";
	}


	void RMeshSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Meshes";

		for (auto [key, uniforms] : mRMeshUniforms) {
			auto [transforms, skin] = mEntityDatabase.getComponents<TransformsComponent, Skin>(key.first);
			if (transforms && transforms->updated.any()) {
				glm::mat4 translation	= glm::translate(glm::mat4(1.0f), transforms->position);
				glm::mat4 rotation		= glm::mat4_cast(transforms->orientation);
				glm::mat4 scale			= glm::scale(glm::mat4(1.0f), transforms->scale);
				glm::mat4 modelMatrix	= translation * rotation * scale;

				uniforms.modelMatrix->setValue(modelMatrix);
				if (skin) {
					auto jointMatrices = calculateJointMatrices(*skin, modelMatrix);
					std::size_t numJoints = std::min(jointMatrices.size(), static_cast<std::size_t>(kMaxJoints));
					uniforms.jointMatrices->setValue(jointMatrices.data(), numJoints);
				}
			}
		}

		SOMBRA_INFO_LOG << "Update end";
	}

}
