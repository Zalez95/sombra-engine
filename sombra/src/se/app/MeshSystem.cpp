#include "se/utils/Log.h"
#include "se/graphics/Technique.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/graphics/core/Program.h"
#include "se/app/MeshSystem.h"
#include "se/app/Application.h"
#include "se/app/SkinComponent.h"
#include "se/app/TransformsComponent.h"

namespace se::app {

	MeshSystem::MeshSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application)
	{
		mApplication.getEventManager()
			.subscribe(this, Topic::RMesh)
			.subscribe(this, Topic::RShader)
			.subscribe(this, Topic::Shader);
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask().set<MeshComponent>());

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


	bool MeshSystem::notify(const IEvent& event)
	{
		return tryCall(&MeshSystem::onRMeshEvent, event)
			|| tryCall(&MeshSystem::onRenderableShaderEvent, event)
			|| tryCall(&MeshSystem::onShaderEvent, event);
	}


	void MeshSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Meshes";

		utils::FixedVector<glm::mat3x4, Skin::kMaxJoints> jointMatrices;

		mEntityDatabase.executeQuery([&](EntityDatabase::Query& query) {
			query.iterateEntityComponents<TransformsComponent, MeshComponent>(
				[&](Entity entity, TransformsComponent* transforms, MeshComponent* mesh) {
					if ((!transforms->updated[static_cast<int>(TransformsComponent::Update::Mesh)]
						|| !transforms->updated[static_cast<int>(TransformsComponent::Update::Skin)])
					) {
						glm::mat4 modelMatrix = getModelMatrix(*transforms);

						mesh->processRenderableIndices([&](std::size_t i) {
							mesh->get(i).setModelMatrix(modelMatrix);
						});

						auto itUniforms = mEntityUniforms.find(entity);
						if (itUniforms != mEntityUniforms.end()) {
							auto [skin] = query.getComponents<SkinComponent>(entity, true);
							if (skin) {
								jointMatrices = skin->calculateJointMatrices(modelMatrix);
							}
							else {
								jointMatrices.clear();
							}

							mesh->processRenderableIndices([&](std::size_t i) {
								for (auto& meshUniforms : itUniforms->second[i]) {
									meshUniforms.modelMatrix->setValue(modelMatrix);
									if (meshUniforms.jointMatrices) {
										meshUniforms.jointMatrices->setValue(jointMatrices.data(), jointMatrices.size());
									}
								}
							});
						}

						transforms->updated.set(static_cast<int>(TransformsComponent::Update::Mesh));
						transforms->updated.set(static_cast<int>(TransformsComponent::Update::Skin));
					}
				},
				true
			);
		});

		SOMBRA_DEBUG_LOG << "Update end";
	}

// Private functions
	void MeshSystem::onNewMesh(Entity entity, MeshComponent* mesh, EntityDatabase::Query& query)
	{
		mesh->setup(&mApplication.getEventManager(), entity);

		auto [transforms] = query.getComponents<TransformsComponent>(entity, true);
		if (transforms) {
			transforms->updated.reset(static_cast<int>(TransformsComponent::Update::Mesh));
		}

		if (mEntityUniforms.emplace(entity, std::array<EntityUniformsVector, MeshComponent::kMaxMeshes>()).second) {
			mesh->processRenderableIndices([&, mesh = mesh](std::size_t i) {
				mesh->processRenderableShaders(i, [&](const auto& shader) {
					shader->processSteps([&](const auto& step) {
						addStep(entity, i, query, step.get());
					});
				});

				mApplication.getExternalTools().graphicsEngine->addRenderable(&mesh->get(i));
			});

			SOMBRA_INFO_LOG << "Entity " << entity << " with MeshComponent " << mesh << " added successfully";
		}
		else {
			SOMBRA_ERROR_LOG << "Failed to add Entity " << entity << " with MeshComponent " << mesh << " to the map";
		}
	}


	void MeshSystem::onRemoveMesh(Entity entity, MeshComponent* mesh, EntityDatabase::Query&)
	{
		mesh->processRenderableIndices([this, mesh = mesh](std::size_t i) {
			mApplication.getExternalTools().graphicsEngine->removeRenderable(&mesh->get(i));
		});

		auto it = mEntityUniforms.find(entity);
		if (it != mEntityUniforms.end()) {
			mEntityUniforms.erase(it);
		}

		mesh->setup(nullptr, kNullEntity);

		SOMBRA_INFO_LOG << "Entity " << entity << " with MeshComponent " << mesh << " removed successfully";
	}


	void MeshSystem::onRMeshEvent(const RMeshEvent& event)
	{
		SOMBRA_INFO_LOG << event;

		mEntityDatabase.executeQuery([&](EntityDatabase::Query& query) {
			auto itEntity = mEntityUniforms.find(event.getEntity());
			if (itEntity == mEntityUniforms.end()) {
				return;
			}

			auto [transforms, mesh] = query.getComponents<TransformsComponent, MeshComponent>(event.getEntity(), true);
			if (mesh) {
				switch (event.getOperation()) {
					case RMeshEvent::Operation::Add: {
						glm::mat4 modelMatrix = (transforms)? getModelMatrix(*transforms) : glm::mat4(1.0f);
						mesh->get(event.getRIndex()).setModelMatrix(modelMatrix);

						mesh->processRenderableShaders(event.getRIndex(), [&](const auto& shader) {
							shader->processSteps([&](const auto& step) {
								addStep(event.getEntity(), event.getRIndex(), query, step.get());
							});
						});

						mApplication.getExternalTools().graphicsEngine->addRenderable(&mesh->get(event.getRIndex()));
					} break;
					case RMeshEvent::Operation::Remove: {
						mApplication.getExternalTools().graphicsEngine->removeRenderable(&mesh->get(event.getRIndex()));
						itEntity->second[event.getRIndex()].clear();
					} break;
				}
			}
		});
	}


	void MeshSystem::onRenderableShaderEvent(const RenderableShaderEvent& event)
	{
		SOMBRA_INFO_LOG << event;

		mEntityDatabase.executeQuery([&](EntityDatabase::Query& query) {
			auto itEntity = mEntityUniforms.find(event.getEntity());
			if ((itEntity == mEntityUniforms.end())
				|| (event.getRComponentType() != RenderableShaderEvent::RComponentType::Mesh)
			) {
				return;
			}

			switch (event.getOperation()) {
				case RenderableShaderEvent::Operation::Add: {
					event.getShader()->processSteps([&](const auto& step) {
						addStep(event.getEntity(), event.getRIndex(), query, step.get());
					});
				} break;
				case RenderableShaderEvent::Operation::Remove: {
					event.getShader()->processSteps([&](const auto& step) {
						removeStep(event.getEntity(), event.getRIndex(), query, step.get());
					});
				} break;
			}
		});
	}


	void MeshSystem::onShaderEvent(const ShaderEvent& event)
	{
		SOMBRA_INFO_LOG << event;

		mEntityDatabase.executeQuery([&](EntityDatabase::Query& query) {
			query.iterateEntityComponents<MeshComponent>(
				[&](Entity entity, MeshComponent* mesh) {
					mesh->processRenderableIndices([&](std::size_t i) {
						bool hasShader = false;
						mesh->processRenderableShaders(i, [&](const auto& shader) {
							hasShader |= (shader.get() == event.getShader());
						});

						if (hasShader) {
							switch (event.getOperation()) {
								case ShaderEvent::Operation::Add: {
									addStep(entity, i, query, event.getStep());
								} break;
								case ShaderEvent::Operation::Remove: {
									removeStep(entity, i, query, event.getStep());
								} break;
							}
						}
					});
				},
				true
			);
		});
	}


	void MeshSystem::addStep(Entity entity, std::size_t rIndex, EntityDatabase::Query& query, const RenderableShaderStepSPtr& step)
	{
		auto [transforms, mesh] = query.getComponents<TransformsComponent, MeshComponent>(entity, true);
		if (!mesh) {
			return;
		}

		// Check if the MeshComponent has the Step already added
		auto& entityUniforms = mEntityUniforms[entity][rIndex];
		auto itUniforms = std::find_if(entityUniforms.begin(), entityUniforms.end(), [&](const auto& uniforms) {
			return uniforms.step == step;
		});
		if (itUniforms != entityUniforms.end()) {
			itUniforms->shaderCount++;
			return;
		}

		// Find the program bindable of the Step
		std::shared_ptr<graphics::Program> program;
		step->processBindables([&](const auto& bindable) {
			if (auto tmp = std::dynamic_pointer_cast<graphics::Program>(bindable)) {
				program = tmp;
			}
		});

		if (!program) {
			SOMBRA_WARN_LOG << "Trying to add a Step " << step.get() << " with no program";
			return;
		}

		// Create and add the uniforms to the mesh
		auto& uniforms = entityUniforms.emplace_back();
		uniforms.shaderCount = 1;
		uniforms.step = step;
		uniforms.modelMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", program);
		if (uniforms.modelMatrix->found()) {
			mesh->get(rIndex).addPassBindable(step->getPass().get(), uniforms.modelMatrix);
		}
		if (mesh->hasSkinning(rIndex)) {
			uniforms.jointMatrices = std::make_shared<graphics::UniformVariableValueVector<glm::mat3x4>>("uJointMatrices", program);
			if (uniforms.jointMatrices->found()) {
				uniforms.jointMatrices->reserve(Skin::kMaxJoints);
				mesh->get(rIndex).addPassBindable(step->getPass().get(), uniforms.jointMatrices);
			}
		}

		if (transforms) {
			transforms->updated.reset(static_cast<int>(TransformsComponent::Update::Mesh));
		}
	}


	void MeshSystem::removeStep(Entity entity, std::size_t rIndex, EntityDatabase::Query& query, const RenderableShaderStepSPtr& step)
	{
		auto [mesh] = query.getComponents<MeshComponent>(entity, true);
		if (!mesh) {
			return;
		}

		auto& entityUniforms = mEntityUniforms.find(entity)->second[rIndex];
		auto itUniforms = std::find_if(entityUniforms.begin(), entityUniforms.end(), [&](const auto& uniforms) {
			return uniforms.step == step;
		});
		if (itUniforms == entityUniforms.end()) {
			return;
		}

		itUniforms->shaderCount--;
		if (itUniforms->shaderCount == 0) {
			mesh->get(rIndex).removePassBindable(step->getPass().get(), itUniforms->modelMatrix);
			if (mesh->hasSkinning(rIndex)) {
				mesh->get(rIndex).removePassBindable(step->getPass().get(), itUniforms->jointMatrices);
			}
			entityUniforms.erase(itUniforms);
		}
	}

}
