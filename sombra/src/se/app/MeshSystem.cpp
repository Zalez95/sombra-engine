#include "se/utils/Log.h"
#include "se/graphics/Technique.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/graphics/core/Program.h"
#include "se/app/MeshSystem.h"
#include "se/app/Application.h"
#include "se/app/MeshComponent.h"
#include "se/app/EntityDatabase.h"
#include "se/app/TransformsComponent.h"

namespace se::app {

	MeshSystem::MeshSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application)
	{
		mApplication.getEventManager()
			.subscribe(this, Topic::RMesh)
			.subscribe(this, Topic::RShader)
			.subscribe(this, Topic::Shader);
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask().set<MeshComponent>().set<SkinComponent>());

		mEntityUniforms.reserve(mEntityDatabase.getMaxComponents<MeshComponent>());
	}


	MeshSystem::~MeshSystem()
	{
		mEntityDatabase.removeSystem(this);
		mApplication.getEventManager()
			.unsubscribe(this, Topic::Shader)
			.unsubscribe(this, Topic::RShader)
			.unsubscribe(this, Topic::RMesh);
	}


	void MeshSystem::notify(const IEvent& event)
	{
		tryCall(&MeshSystem::onRMeshEvent, event);
		tryCall(&MeshSystem::onRenderableShaderEvent, event);
		tryCall(&MeshSystem::onShaderEvent, event);
	}


	void MeshSystem::onNewEntity(Entity entity)
	{
		auto [transforms, mesh, skin] = mEntityDatabase.getComponents<TransformsComponent, MeshComponent, SkinComponent>(entity);
		if (!mesh) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as Mesh";
			return;
		}

		glm::mat4 modelMatrix = (transforms)? getModelMatrix(*transforms) : glm::mat4(1.0f);

		auto itEntity = mEntityUniforms.emplace(entity, EntityUniformsVector()).first;
		for (std::size_t i = 0; i < mesh->size(); ++i) {
			mesh->get(i).setModelMatrix(modelMatrix);

			itEntity->second.emplace_back();
			mesh->processRenderableShaders(i, [&](const RenderableShaderSPtr& shader) {
				shader->getTechnique()->processPasses([&](const PassSPtr& pass) {
					addPass(entity, i, pass);
				});
			});

			mApplication.getExternalTools().graphicsEngine->addRenderable(&mesh->get(i));
		}

		SOMBRA_INFO_LOG << "Entity " << entity << " with MeshComponent " << mesh << " added successfully";
	}


	void MeshSystem::onRemoveEntity(Entity entity)
	{
		auto [mesh] = mEntityDatabase.getComponents<MeshComponent>(entity);
		if (!mesh) {
			SOMBRA_INFO_LOG << "Mesh Entity " << entity << " couldn't removed";
			return;
		}

		for (std::size_t i = 0; i < mesh->size(); ++i) {
			mApplication.getExternalTools().graphicsEngine->removeRenderable(&mesh->get(i));
		}

		auto it = mEntityUniforms.find(entity);
		if (it != mEntityUniforms.end()) {
			mEntityUniforms.erase(it);
		}

		SOMBRA_INFO_LOG << "Mesh Entity " << entity << " removed successfully";
	}


	void MeshSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Meshes";

		for (auto& [entity, entityUniforms] : mEntityUniforms) {
			auto [transforms, mesh, skin] = mEntityDatabase.getComponents<TransformsComponent, MeshComponent, SkinComponent>(entity);
			if (transforms && transforms->updated.any()) {
				glm::mat4 modelMatrix	= getModelMatrix(*transforms);

				//FIXME: joints updated but no transforms
				utils::FixedVector<glm::mat3x4, Skin::kMaxJoints> jointMatrices;
				if (skin) {
					jointMatrices = skin->calculateJointMatrices(modelMatrix);
				}
				std::size_t numJoints = std::min(jointMatrices.size(), static_cast<std::size_t>(Skin::kMaxJoints));

				for (std::size_t i = 0; i < mesh->size(); ++i) {
					mesh->get(i).setModelMatrix(modelMatrix);

					for (auto& meshUniforms : entityUniforms[i]) {
						meshUniforms.modelMatrix->setValue(modelMatrix);
						if (meshUniforms.jointMatrices) {
							meshUniforms.jointMatrices->setValue(jointMatrices.data(), numJoints);
						}
					}
				}
			}
		}

		SOMBRA_INFO_LOG << "Update end";
	}

// Private functions
	void MeshSystem::onRMeshEvent(const RMeshEvent& event)
	{
		auto itEntity = mEntityUniforms.find(event.getEntity());
		if (itEntity == mEntityUniforms.end()) {
			return;
		}

		auto [transforms, mesh] = mEntityDatabase.getComponents<TransformsComponent, MeshComponent>(event.getEntity());

		switch (event.getOperation()) {
			case RMeshEvent::Operation::Add: {
				glm::mat4 modelMatrix = (transforms)? getModelMatrix(*transforms) : glm::mat4(1.0f);
				mesh->get(event.getRIndex()).setModelMatrix(modelMatrix);

				itEntity->second.emplace_back();
				mesh->processRenderableShaders(event.getRIndex(), [&](const RenderableShaderSPtr& shader) {
					shader->getTechnique()->processPasses([&](const PassSPtr& pass) {
						addPass(event.getEntity(), event.getRIndex(), pass);
					});
				});

				mApplication.getExternalTools().graphicsEngine->addRenderable(&mesh->get(event.getRIndex()));
				assert(itEntity->second.size() - 1 == event.getRIndex() && "rIndex doesn't match");
			} break;
			case RMeshEvent::Operation::Remove: {
				mApplication.getExternalTools().graphicsEngine->removeRenderable(&mesh->get(event.getRIndex()));
				itEntity->second.erase(itEntity->second.begin() + event.getRIndex());
			} break;
		}
	}


	void MeshSystem::onRenderableShaderEvent(const RenderableShaderEvent& event)
	{
		auto itEntity = mEntityUniforms.find(event.getEntity());
		if (itEntity == mEntityUniforms.end() || event.isTerrain()) {
			return;
		}

		switch (event.getOperation()) {
			case RenderableShaderEvent::Operation::Add: {
				event.getShader()->getTechnique()->processPasses([&](const PassSPtr& pass) {
					addPass(event.getEntity(), event.getRIndex(), pass);
				});
			} break;
			case RenderableShaderEvent::Operation::Remove: {
				event.getShader()->getTechnique()->processPasses([&](const PassSPtr& pass) {
					removePass(event.getEntity(), event.getRIndex(), pass);
				});
			} break;
		}
	}


	void MeshSystem::onShaderEvent(const ShaderEvent& event)
	{
		mEntityDatabase.iterateComponents<MeshComponent>([&](Entity entity, MeshComponent* mesh) {
			for (std::size_t i = 0; i < mesh->size(); ++i) {
				bool hasShader = false;
				mesh->processRenderableShaders(i, [&](const RenderableShaderSPtr& shader) {
					hasShader |= (shader == event.getShader());
				});

				if (hasShader) {
					switch (event.getOperation()) {
						case ShaderEvent::Operation::Add: {
							addPass(entity, i, event.getPass());
						} break;
						case ShaderEvent::Operation::Remove: {
							removePass(entity, i, event.getPass());
						} break;
					}
				}
			}
		});
	}


	void MeshSystem::addPass(Entity entity, std::size_t rIndex, const PassSPtr& pass)
	{
		auto [transforms, mesh, skin] = mEntityDatabase.getComponents<TransformsComponent, MeshComponent, SkinComponent>(entity);
		if (!mesh) {
			return;
		}

		// Check if the MeshComponent has the Pass already added
		auto& entityUniforms = mEntityUniforms.find(entity)->second[rIndex];
		auto itUniforms = std::find_if(entityUniforms.begin(), entityUniforms.end(), [&](const auto& uniforms) {
			return uniforms.pass == pass;
		});
		if (itUniforms != entityUniforms.end()) {
			itUniforms->shaderCount++;
			return;
		}

		// Find the program bindable of the Pass
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

		// Create and add the uniforms to the mesh
		glm::mat4 modelMatrix = (transforms)? getModelMatrix(*transforms) : glm::mat4(1.0f);

		auto& uniforms = entityUniforms.emplace_back();
		uniforms.shaderCount = 1;
		uniforms.pass = pass;
		uniforms.modelMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>(
			"uModelMatrix", *program, modelMatrix
		);
		if (uniforms.modelMatrix->found()) {
			mesh->get(rIndex).addPassBindable(pass.get(), uniforms.modelMatrix);
		}
		if (mesh->hasSkinning(rIndex)) {
			utils::FixedVector<glm::mat3x4, Skin::kMaxJoints> jointMatrices;
			if (skin) {
				jointMatrices = skin->calculateJointMatrices(modelMatrix);
			}
			std::size_t numJoints = std::min(jointMatrices.size(), static_cast<std::size_t>(Skin::kMaxJoints));

			uniforms.jointMatrices = std::make_shared<graphics::UniformVariableValueVector<glm::mat3x4, Skin::kMaxJoints>>(
				"uJointMatrices", *program, jointMatrices.data(), numJoints
			);
			if (uniforms.jointMatrices->found()) {
				mesh->get(rIndex).addPassBindable(pass.get(), uniforms.jointMatrices);
			}
		}
	}


	void MeshSystem::removePass(Entity entity, std::size_t rIndex, const PassSPtr& pass)
	{
		auto [mesh] = mEntityDatabase.getComponents<MeshComponent>(entity);
		if (!mesh) {
			return;
		}

		auto& entityUniforms = mEntityUniforms.find(entity)->second[rIndex];
		auto itUniforms = std::find_if(entityUniforms.begin(), entityUniforms.end(), [&](const auto& uniforms) {
			return uniforms.pass == pass;
		});
		if (itUniforms == entityUniforms.end()) {
			return;
		}

		itUniforms->shaderCount--;
		if (itUniforms->shaderCount == 0) {
			mesh->get(rIndex).removePassBindable(pass.get(), itUniforms->modelMatrix);
			if (mesh->hasSkinning(rIndex)) {
				mesh->get(rIndex).removePassBindable(pass.get(), itUniforms->jointMatrices);
			}
			entityUniforms.erase(itUniforms);
		}
	}

}
