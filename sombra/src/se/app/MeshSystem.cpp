#include "se/utils/Log.h"
#include "se/utils/ThreadPool.h"
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
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask()
			.set<MeshComponent>()
			.set<TransformsComponent>()
		);

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


	void MeshSystem::onNewComponent(
		Entity entity, const EntityDatabase::ComponentMask& mask,
		EntityDatabase::Query& query
	) {
		tryCallC(&MeshSystem::onNewMesh, entity, mask, query);
		tryCallC(&MeshSystem::onNewTransforms, entity, mask, query);
	}


	void MeshSystem::onRemoveComponent(
		Entity entity, const EntityDatabase::ComponentMask& mask,
		EntityDatabase::Query& query
	) {
		tryCallC(&MeshSystem::onRemoveMesh, entity, mask, query);
	}


	void MeshSystem::update(float, float)
	{
		SOMBRA_DEBUG_LOG << "Updating the Meshes";

		mEntityDatabase.executeQuery([this](EntityDatabase::Query& query) {
			std::unique_lock lock(mUniformsMutex);
			SOMBRA_DEBUG_LOG << "Adding new uniforms";

			std::queue<NewUniform> nextNewUniforms;
			while (!mNewUniforms.empty()) {
				NewUniform newUniform = std::move(mNewUniforms.front());
				mNewUniforms.pop();
				lock.unlock();

				if (utils::is_ready(newUniform.uniformFound)) {
					auto [transforms, mesh] = query.getComponents<TransformsComponent, MeshComponent>(newUniform.entity, true);
					bool found = newUniform.uniformFound.get();
					auto modelMatrix = UniformVVRef<glm::mat4>::from(newUniform.uniform);
					auto jointMatrices = UniformVVVRef<glm::mat3x4>::from(newUniform.uniform);

					if (mesh && mesh->isActive(newUniform.rIndex) && found) {
						std::scoped_lock lock2(mMutex);
						auto itEntity = mEntityUniforms.find(newUniform.entity);
						if (itEntity != mEntityUniforms.end()) {
							auto& entityUniforms = itEntity->second[newUniform.rIndex];
							auto itUniforms = std::find_if(entityUniforms.begin(), entityUniforms.end(), [&](const auto& uniforms) {
								return uniforms.step == newUniform.step;
							});
							if (itUniforms != entityUniforms.end()) {
								if (modelMatrix) {
									itUniforms->modelMatrix = modelMatrix;
									mesh->get(newUniform.rIndex).addPassBindable(itUniforms->step->getPass().get(), modelMatrix);
								}

								if (jointMatrices) {
									itUniforms->jointMatrices = jointMatrices;
									mesh->get(newUniform.rIndex).addPassBindable(itUniforms->step->getPass().get(), jointMatrices);
								}

								if (transforms) {
									transforms->updated.reset(static_cast<int>(TransformsComponent::Update::Mesh));
								}
							}
						}
					}
				}
				else {
					nextNewUniforms.push(std::move(newUniform));
				}

				lock.lock();
			}

			std::swap(mNewUniforms, nextNewUniforms);
		});


		mEntityDatabase.executeQuery([this](EntityDatabase::Query& query) {
			SOMBRA_DEBUG_LOG << "Updating model and joint matrices";

			utils::FixedVector<glm::mat3x4, Skin::kMaxJoints> jointMatrices;
			query.iterateEntityComponents<TransformsComponent, MeshComponent>(
				[&](Entity entity, TransformsComponent* transforms, MeshComponent* mesh) {
					if ((!transforms->updated[static_cast<int>(TransformsComponent::Update::Mesh)]
						|| !transforms->updated[static_cast<int>(TransformsComponent::Update::Skin)])
					) {
						glm::mat4 modelMatrix = getModelMatrix(*transforms);

						mesh->processRenderableIndices([&](std::size_t i) {
							mesh->get(i).setModelMatrix(modelMatrix);
						});

						std::scoped_lock lock(mMutex);
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
									if (meshUniforms.modelMatrix) {
										meshUniforms.modelMatrix.edit([=](auto& uniform) { uniform.setValue(modelMatrix); });
									}
									if (meshUniforms.jointMatrices) {
										meshUniforms.jointMatrices.edit([=](auto& uniform) { uniform.setValue(jointMatrices.data(), jointMatrices.size()); });
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

		{
			std::scoped_lock lock(mMutex);
			mEntityUniforms.emplace(entity, std::array<EntityUniformsVector, MeshComponent::kMaxMeshes>());
		}

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


	void MeshSystem::onRemoveMesh(Entity entity, MeshComponent* mesh, EntityDatabase::Query&)
	{
		mesh->processRenderableIndices([this, mesh = mesh](std::size_t i) {
			mApplication.getExternalTools().graphicsEngine->removeRenderable(&mesh->get(i));
		});

		std::scoped_lock lock(mMutex);
		auto it = mEntityUniforms.find(entity);
		if (it != mEntityUniforms.end()) {
			mEntityUniforms.erase(it);
		}

		mesh->setup(nullptr, kNullEntity);

		SOMBRA_INFO_LOG << "Entity " << entity << " with MeshComponent " << mesh << " removed successfully";
	}


	void MeshSystem::onNewTransforms(Entity, TransformsComponent* transforms, EntityDatabase::Query&)
	{
		transforms->updated.reset(static_cast<int>(TransformsComponent::Update::Mesh));
	}


	void MeshSystem::onRMeshEvent(const RMeshEvent& event)
	{
		SOMBRA_INFO_LOG << event;

		mEntityDatabase.executeQuery([&](EntityDatabase::Query& query) {
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

						std::scoped_lock lock(mMutex);
						auto itEntity = mEntityUniforms.find(event.getEntity());
						if (itEntity != mEntityUniforms.end()) {
							itEntity->second[event.getRIndex()].clear();
						}
					} break;
				}
			}
		});
	}


	void MeshSystem::onRenderableShaderEvent(const RenderableShaderEvent& event)
	{
		SOMBRA_INFO_LOG << event;

		if (event.getRComponentType() != RenderableShaderEvent::RComponentType::Mesh) {
			return;
		}

		mEntityDatabase.executeQuery([&](EntityDatabase::Query& query) {
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
		auto [mesh] = query.getComponents<MeshComponent>(entity, true);
		if (!mesh) {
			return;
		}

		{	// Check if the mesh has the Step already added
			std::scoped_lock lock(mMutex);
			auto& entityUniforms = mEntityUniforms.find(entity)->second[rIndex];
			auto itUniforms = std::find_if(entityUniforms.begin(), entityUniforms.end(), [&](const auto& uniforms) {
				return uniforms.step == step;
			});
			if (itUniforms != entityUniforms.end()) {
				itUniforms->shaderCount++;
				return;
			}
			else {
				entityUniforms.push_back({ 1, step, UniformVVRef<glm::mat4>(), UniformVVVRef<glm::mat3x4>() });
			}
		}

		// Find the program bindable of the Step
		ProgramRef program;
		step->processPrograms([&](const auto& program2) {
			program = *program2;
		});

		if (!program) {
			SOMBRA_WARN_LOG << "Trying to add a Step " << step.get() << " with no program";
			return;
		}

		// Create the uniforms
		auto& context = mApplication.getExternalTools().graphicsEngine->getContext();

		{
			auto p = std::make_shared<std::promise<bool>>();
			auto modelMatrixFound = p->get_future();

			auto modelMatrix = context.create<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix")
				.qedit([=](auto& q, auto& uniform) {
					p->set_value( uniform.load(*q.getTBindable(program)) );
				});

			std::scoped_lock lock(mUniformsMutex);
			mNewUniforms.push({ entity, rIndex, step, std::move(modelMatrix), std::move(modelMatrixFound) });
		}

		if (mesh->hasSkinning(rIndex)) {
			auto p = std::make_shared<std::promise<bool>>();
			auto jointMatricesFound = p->get_future();

			auto jointMatrices = context.create<graphics::UniformVariableValueVector<glm::mat3x4>>("uJointMatrices")
				.qedit([=](auto& q, auto& uniform) {
					p->set_value( uniform.load(*q.getTBindable(program)) );
				});

			std::scoped_lock lock(mUniformsMutex);
			mNewUniforms.push({ entity, rIndex, step, std::move(jointMatrices), std::move(jointMatricesFound) });
		}
	}


	void MeshSystem::removeStep(Entity entity, std::size_t rIndex, EntityDatabase::Query& query, const RenderableShaderStepSPtr& step)
	{
		auto [mesh] = query.getComponents<MeshComponent>(entity, true);
		if (!mesh) {
			return;
		}

		std::scoped_lock lock(mMutex);
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
