#include "se/utils/Log.h"
#include "se/graphics/Technique.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/3D/RenderableTerrain.h"
#include "se/app/RTerrainSystem.h"
#include "se/app/Application.h"
#include "se/app/EntityDatabase.h"
#include "se/app/TransformsComponent.h"

namespace se::app {

	RTerrainSystem::RTerrainSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application),
		mCameraEntity(kNullEntity), mCameraUpdated(false)
	{
		mApplication.getEventManager().subscribe(this, Topic::Camera);
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask().set<graphics::RenderableTerrain>());
	}


	RTerrainSystem::~RTerrainSystem()
	{
		mEntityDatabase.removeSystem(this);
		mApplication.getEventManager().unsubscribe(this, Topic::Camera);
	}


	void RTerrainSystem::notify(const IEvent& event)
	{
		tryCall(&RTerrainSystem::onCameraEvent, event);
	}


	void RTerrainSystem::onNewEntity(Entity entity)
	{
		auto [transforms, rTerrain] = mEntityDatabase.getComponents<TransformsComponent, graphics::RenderableTerrain>(entity);
		if (!rTerrain) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as Terrain";
			return;
		}

		glm::mat4 modelMatrix(1.0f);
		if (transforms) {
			glm::mat4 translation	= glm::translate(glm::mat4(1.0f), transforms->position);
			glm::mat4 rotation		= glm::mat4_cast(transforms->orientation);
			modelMatrix = translation * rotation;
		}

		auto& entityUniforms = mEntityUniforms[entity];
		rTerrain->processTechniques([&, rTerrain = rTerrain](auto technique) { technique->processPasses([&](auto pass) {
			std::shared_ptr<graphics::Program> program;
			pass->processBindables([&](const auto& bindable) {
				if (auto tmp = std::dynamic_pointer_cast<graphics::Program>(bindable)) {
					program = tmp;
				}
			});

			if (program) {
				auto& terrainUniforms = entityUniforms.emplace_back();
				terrainUniforms.pass = pass;
				terrainUniforms.modelMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", *program, modelMatrix);
				rTerrain->addPassBindable(pass.get(), terrainUniforms.modelMatrix);
			}
			else {
				SOMBRA_WARN_LOG << "RenderableTerrain has a Pass " << pass << " with no program";
			}
		}); });

		auto [camTransforms] = mEntityDatabase.getComponents<TransformsComponent>(mCameraEntity);
		rTerrain->setHighestLodLocation(camTransforms? camTransforms->position : glm::vec3(0.0f));

		mApplication.getExternalTools().graphicsEngine->addRenderable(rTerrain);
		SOMBRA_INFO_LOG << "Entity " << entity << " with RenderableTerrain " << rTerrain << " added successfully";
	}


	void RTerrainSystem::onRemoveEntity(Entity entity)
	{
		auto [rTerrain] = mEntityDatabase.getComponents<graphics::RenderableTerrain>(entity);
		if (!rTerrain) {
			SOMBRA_INFO_LOG << "Terrain Entity " << entity << " couldn't removed";
			return;
		}

		mApplication.getExternalTools().graphicsEngine->removeRenderable(rTerrain);

		auto it = mEntityUniforms.find(entity);
		if (it != mEntityUniforms.end()) {
			for (auto& uniforms : it->second) {
				rTerrain->removePassBindable(uniforms.pass.get(), uniforms.modelMatrix);
			}
			mEntityUniforms.erase(it);
		}

		SOMBRA_INFO_LOG << "Terrain Entity " << entity << " removed successfully";
	}


	void RTerrainSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Terrains";

		glm::vec3 camPosition(0.0f);
		auto [camTransforms] = mEntityDatabase.getComponents<TransformsComponent>(mCameraEntity);
		if (camTransforms && (camTransforms->updated.any() || mCameraUpdated)) {
			mCameraUpdated = true;
			camPosition = camTransforms->position;
		}

		for (auto [entity, entityUniforms] : mEntityUniforms) {
			auto [transforms, rTerrain] = mEntityDatabase.getComponents<TransformsComponent, graphics::RenderableTerrain>(entity);
			if (transforms && transforms->updated.any()) {
				glm::mat4 translation	= glm::translate(glm::mat4(1.0f), transforms->position);
				glm::mat4 rotation		= glm::mat4_cast(transforms->orientation);
				glm::mat4 modelMatrix	= translation * rotation;

				for (auto& terrainUniforms : entityUniforms) {
					terrainUniforms.modelMatrix->setValue(modelMatrix);
				}
			}

			if (mCameraUpdated) {
				rTerrain->setHighestLodLocation(camPosition);
			}
		}

		mCameraUpdated = false;

		SOMBRA_INFO_LOG << "Update end";
	}

// Private functions
	void RTerrainSystem::onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event)
	{
		mCameraEntity = event.getValue();
		mCameraUpdated = true;
	}

}
