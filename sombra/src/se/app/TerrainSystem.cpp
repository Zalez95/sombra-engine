#include "se/utils/Log.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/graphics/core/Program.h"
#include "se/app/TerrainSystem.h"
#include "se/app/Application.h"
#include "se/app/TransformsComponent.h"
#include "se/app/TerrainComponent.h"

namespace se::app {

	TerrainSystem::TerrainSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application),
		mCameraEntity(kNullEntity), mCameraUpdated(false)
	{
		mApplication.getEventManager()
			.subscribe(this, Topic::Camera)
			.subscribe(this, Topic::RShader)
			.subscribe(this, Topic::Shader);
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask().set<TerrainComponent>());

		mEntityUniforms.reserve(mEntityDatabase.getMaxComponents<TerrainComponent>());
	}


	TerrainSystem::~TerrainSystem()
	{
		mEntityDatabase.removeSystem(this);
		mApplication.getEventManager()
			.unsubscribe(this, Topic::Shader)
			.unsubscribe(this, Topic::RShader)
			.unsubscribe(this, Topic::Camera);
	}


	bool TerrainSystem::notify(const IEvent& event)
	{
		return tryCall(&TerrainSystem::onCameraEvent, event)
			|| tryCall(&TerrainSystem::onRenderableShaderEvent, event)
			|| tryCall(&TerrainSystem::onShaderEvent, event);
	}


	void TerrainSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Terrains";

		glm::vec3 camPosition(0.0f);
		mEntityDatabase.executeQuery([&](EntityDatabase::Query& query) {
			auto [camTransforms] = query.getComponents<TransformsComponent>(mCameraEntity, true);
			if (camTransforms && (camTransforms->updated.any() || mCameraUpdated)) {
				mCameraUpdated = true;
				camPosition = camTransforms->position;
			}
		});

		mEntityDatabase.executeQuery([&](EntityDatabase::Query& query) {
			query.iterateEntityComponents<TransformsComponent, TerrainComponent>(
				[&](Entity entity, TransformsComponent* transforms, TerrainComponent* terrain) {
					if (!transforms->updated[static_cast<int>(TransformsComponent::Update::Terrain)]) {
						glm::mat4 translation	= glm::translate(glm::mat4(1.0f), transforms->position);
						glm::mat4 rotation		= glm::mat4_cast(transforms->orientation);
						glm::mat4 modelMatrix	= translation * rotation;

						terrain->get().setModelMatrix(modelMatrix);

						auto itUniforms = mEntityUniforms.find(entity);
						if (itUniforms != mEntityUniforms.end()) {
							for (auto& uniforms : itUniforms->second) {
								uniforms.modelMatrix->setValue(modelMatrix);
							}
						}

						transforms->updated.set(static_cast<int>(TransformsComponent::Update::Terrain));
					}

					if (mCameraUpdated) {
						terrain->get().setHighestLodLocation(camPosition);
					}
				},
				true
			);
		});

		mCameraUpdated = false;

		SOMBRA_DEBUG_LOG << "Update end";
	}

// Private functions
	void TerrainSystem::onNewTerrain(Entity entity, TerrainComponent* terrain, EntityDatabase::Query& query)
	{
		terrain->setup(&mApplication.getEventManager(), entity);

		auto [transforms] = query.getComponents<TransformsComponent>(entity, true);
		if (transforms) {
			transforms->updated.reset(static_cast<int>(TransformsComponent::Update::Terrain));
		}

		auto [camTransforms] = query.getComponents<TransformsComponent>(mCameraEntity, true);
		if (camTransforms) {
			terrain->get().setHighestLodLocation(camTransforms->position);
		}

		mEntityUniforms.emplace(entity, EntityUniformsVector());
		terrain->processRenderableShaders([&](const auto& shader) {
			shader->processSteps([&](const auto& step) {
				addStep(entity, query, step.get());
			});
		});

		mApplication.getExternalTools().graphicsEngine->addRenderable(&terrain->get());
		SOMBRA_INFO_LOG << "Entity " << entity << " with RenderableTerrain " << terrain << " added successfully";
	}


	void TerrainSystem::onRemoveTerrain(Entity entity, TerrainComponent* terrain, EntityDatabase::Query&)
	{
		mApplication.getExternalTools().graphicsEngine->removeRenderable(&terrain->get());

		auto it = mEntityUniforms.find(entity);
		if (it != mEntityUniforms.end()) {
			mEntityUniforms.erase(it);
		}

		terrain->setup(nullptr, kNullEntity);

		SOMBRA_INFO_LOG << "Entity " << entity << " with RenderableTerrain " << terrain << " removed successfully";
	}


	void TerrainSystem::onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event)
	{
		SOMBRA_INFO_LOG << event;

		mCameraEntity = event.getValue();
		mCameraUpdated = true;
		SOMBRA_INFO_LOG << "Entity " << mCameraEntity << " setted as camera";
	}


	void TerrainSystem::onRenderableShaderEvent(const RenderableShaderEvent& event)
	{
		SOMBRA_INFO_LOG << event;

		mEntityDatabase.executeQuery([&](EntityDatabase::Query& query) {
			auto itEntity = mEntityUniforms.find(event.getEntity());
			if ((itEntity == mEntityUniforms.end())
				|| (event.getRComponentType() != RenderableShaderEvent::RComponentType::Terrain)
			) {
				return;
			}

			switch (event.getOperation()) {
				case RenderableShaderEvent::Operation::Add: {
					event.getShader()->processSteps([&](const auto& step) {
						addStep(event.getEntity(), query, step.get());
					});
				} break;
				case RenderableShaderEvent::Operation::Remove: {
					event.getShader()->processSteps([&](const auto& step) {
						removeStep(event.getEntity(), query, step.get());
					});
				} break;
			}
		});
	}


	void TerrainSystem::onShaderEvent(const ShaderEvent& event)
	{
		SOMBRA_INFO_LOG << event;

		mEntityDatabase.executeQuery([&](EntityDatabase::Query& query) {
			query.iterateEntityComponents<TerrainComponent>(
				[&](Entity entity, TerrainComponent* terrain) {
					bool hasShader = false;
					terrain->processRenderableShaders([&](const auto& shader) {
						hasShader |= (shader.get() == event.getShader());
					});

					if (hasShader) {
						switch (event.getOperation()) {
							case ShaderEvent::Operation::Add: {
								addStep(entity, query, event.getStep());
							} break;
							case ShaderEvent::Operation::Remove: {
								removeStep(entity, query, event.getStep());
							} break;
						}
					}
				},
				true
			);
		});
	}


	void TerrainSystem::addStep(Entity entity, EntityDatabase::Query& query, const RenderableShaderStepSPtr& step)
	{
		auto [transforms, terrain] = query.getComponents<TransformsComponent, TerrainComponent>(entity, true);
		if (!terrain) {
			return;
		}

		// Check if the terrain has the Step already added
		auto& entityUniforms = mEntityUniforms.find(entity)->second;
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

		// Create and add the uniforms to the terrain
		auto& uniforms = entityUniforms.emplace_back();
		uniforms.shaderCount = 1;
		uniforms.step = step;
		uniforms.modelMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", program);
		if (uniforms.modelMatrix->found()) {
			terrain->get().addPassBindable(step->getPass().get(), uniforms.modelMatrix);
		}

		if (transforms) {
			transforms->updated.reset(static_cast<int>(TransformsComponent::Update::Terrain));
		}
	}


	void TerrainSystem::removeStep(Entity entity, EntityDatabase::Query& query, const RenderableShaderStepSPtr& step)
	{
		auto [terrain] = query.getComponents<TerrainComponent>(entity, true);
		if (!terrain) {
			return;
		}

		auto& entityUniforms = mEntityUniforms.find(entity)->second;
		auto itUniforms = std::find_if(entityUniforms.begin(), entityUniforms.end(), [&](const auto& uniforms) {
			return uniforms.step == step;
		});
		if (itUniforms == entityUniforms.end()) {
			return;
		}

		itUniforms->shaderCount--;
		if (itUniforms->shaderCount == 0) {
			terrain->get().removePassBindable(step->getPass().get(), itUniforms->modelMatrix);
			entityUniforms.erase(itUniforms);
		}
	}

}
