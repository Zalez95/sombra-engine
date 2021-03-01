#include "se/utils/Log.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/graphics/core/Program.h"
#include "se/app/TerrainSystem.h"
#include "se/app/Application.h"
#include "se/app/EntityDatabase.h"
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


	void TerrainSystem::notify(const IEvent& event)
	{
		tryCall(&TerrainSystem::onCameraEvent, event);
		tryCall(&TerrainSystem::onRenderableShaderEvent, event);
		tryCall(&TerrainSystem::onShaderEvent, event);
	}


	void TerrainSystem::onNewEntity(Entity entity)
	{
		auto [transforms, terrain] = mEntityDatabase.getComponents<TransformsComponent, TerrainComponent>(entity);
		if (!terrain) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as Terrain";
			return;
		}

		if (transforms) {
			transforms->updated.reset(static_cast<int>(TransformsComponent::Update::Terrain));
		}

		auto [camTransforms] = mEntityDatabase.getComponents<TransformsComponent>(mCameraEntity);
		if (camTransforms) {
			terrain->get().setHighestLodLocation(camTransforms->position);
		}

		mEntityUniforms.emplace(entity, EntityUniformsVector());
		terrain->processRenderableShaders([&](const RenderableShaderSPtr& shader) {
			shader->getTechnique()->processPasses([&](const PassSPtr& pass) {
				addPass(entity, pass);
			});
		});

		mApplication.getExternalTools().graphicsEngine->addRenderable(&terrain->get());
		SOMBRA_INFO_LOG << "Entity " << entity << " with RenderableTerrain " << terrain << " added successfully";
	}


	void TerrainSystem::onRemoveEntity(Entity entity)
	{
		auto [terrain] = mEntityDatabase.getComponents<TerrainComponent>(entity);
		if (!terrain) {
			SOMBRA_INFO_LOG << "Terrain Entity " << entity << " couldn't removed";
			return;
		}

		mApplication.getExternalTools().graphicsEngine->removeRenderable(&terrain->get());

		auto it = mEntityUniforms.find(entity);
		if (it != mEntityUniforms.end()) {
			mEntityUniforms.erase(it);
		}

		SOMBRA_INFO_LOG << "Terrain Entity " << entity << " removed successfully";
	}


	void TerrainSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Terrains";

		glm::vec3 camPosition(0.0f);
		auto [camTransforms] = mEntityDatabase.getComponents<TransformsComponent>(mCameraEntity);
		if (camTransforms && (camTransforms->updated.any() || mCameraUpdated)) {
			mCameraUpdated = true;
			camPosition = camTransforms->position;
		}

		for (auto& [entity, entityUniforms] : mEntityUniforms) {
			auto [transforms, terrain] = mEntityDatabase.getComponents<TransformsComponent, TerrainComponent>(entity);
			if (transforms && !transforms->updated[static_cast<int>(TransformsComponent::Update::Terrain)]) {
				glm::mat4 translation	= glm::translate(glm::mat4(1.0f), transforms->position);
				glm::mat4 rotation		= glm::mat4_cast(transforms->orientation);
				glm::mat4 modelMatrix	= translation * rotation;

				terrain->get().setModelMatrix(modelMatrix);
				for (auto& uniforms : entityUniforms) {
					uniforms.modelMatrix->setValue(modelMatrix);
				}

				transforms->updated.set(static_cast<int>(TransformsComponent::Update::Terrain));
			}

			if (mCameraUpdated) {
				terrain->get().setHighestLodLocation(camPosition);
			}
		}

		mCameraUpdated = false;

		SOMBRA_INFO_LOG << "Update end";
	}

// Private functions
	void TerrainSystem::onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event)
	{
		mCameraEntity = event.getValue();
		mCameraUpdated = true;
	}


	void TerrainSystem::onRenderableShaderEvent(const RenderableShaderEvent& event)
	{
		auto itEntity = mEntityUniforms.find(event.getEntity());
		if (itEntity == mEntityUniforms.end() || !event.isTerrain()) {
			return;
		}

		switch (event.getOperation()) {
			case RenderableShaderEvent::Operation::Add: {
				event.getShader()->getTechnique()->processPasses([&](const PassSPtr& pass) {
					addPass(event.getEntity(), pass);
				});
			} break;
			case RenderableShaderEvent::Operation::Remove: {
				event.getShader()->getTechnique()->processPasses([&](const PassSPtr& pass) {
					removePass(event.getEntity(), pass);
				});
			} break;
		}
	}


	void TerrainSystem::onShaderEvent(const ShaderEvent& event)
	{
		mEntityDatabase.iterateComponents<TerrainComponent>([&](Entity entity, TerrainComponent* terrain) {
			bool hasShader = false;
			terrain->processRenderableShaders([&](const RenderableShaderSPtr& shader) {
				hasShader |= (shader == event.getShader());
			});

			if (hasShader) {
				switch (event.getOperation()) {
					case ShaderEvent::Operation::Add: {
						addPass(entity, event.getPass());
					} break;
					case ShaderEvent::Operation::Remove: {
						removePass(entity, event.getPass());
					} break;
				}
			}
		});
	}


	void TerrainSystem::addPass(Entity entity, const PassSPtr& pass)
	{
		auto [transforms, terrain] = mEntityDatabase.getComponents<TransformsComponent, TerrainComponent>(entity);
		if (!terrain) {
			return;
		}

		// Check if the terrain has the Pass already added
		auto& entityUniforms = mEntityUniforms.find(entity)->second;
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

		// Create and add the uniforms to the terrain
		auto& uniforms = entityUniforms.emplace_back();
		uniforms.shaderCount = 1;
		uniforms.pass = pass;
		uniforms.modelMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", *program);
		if (uniforms.modelMatrix->found()) {
			terrain->get().addPassBindable(pass.get(), uniforms.modelMatrix);
		}

		if (transforms) {
			transforms->updated.reset(static_cast<int>(TransformsComponent::Update::Terrain));
		}
	}


	void TerrainSystem::removePass(Entity entity, const PassSPtr& pass)
	{
		auto [terrain] = mEntityDatabase.getComponents<TerrainComponent>(entity);
		if (!terrain) {
			return;
		}

		auto& entityUniforms = mEntityUniforms.find(entity)->second;
		auto itUniforms = std::find_if(entityUniforms.begin(), entityUniforms.end(), [&](const auto& uniforms) {
			return uniforms.pass == pass;
		});
		if (itUniforms == entityUniforms.end()) {
			return;
		}

		itUniforms->shaderCount--;
		if (itUniforms->shaderCount == 0) {
			terrain->get().removePassBindable(pass.get(), itUniforms->modelMatrix);
			entityUniforms.erase(itUniforms);
		}
	}

}
